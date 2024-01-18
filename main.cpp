// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <iostream>

#include "mem/mem.hpp"
#include "hack/hack.hpp"

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(400, 300, "Dontna's WaW Trainer", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    //* Hack Variables *//
    if (mem::GetPIDFromProcessName(hackVariables.processName) == 1)
    {
        std::cout << "Cannot find the game, is it running?\n";
        return 0;
    }

    hackVariables.processID = mem::GetPIDFromProcessName(hackVariables.processName);

    int sliderValue = 0;

    // Spawn our new thread
    std::thread hackThread(HackThread);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // If process isn't alive anymore, exit.
        if (mem::GetPIDFromProcessName(hackVariables.processName) == 1)
        {
            std::cout << "Cannot find the game, is it running?\n";
            hackVariables.hackShouldStop = true;
            hackThread.join();
            return 0;
        }

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Our GUI code
        if (ImGui::Begin("Main Menu", NULL))
        {
            ImGui::Text("\t\t\tCoDWaW - Zombies Trainer");
            if(ImGui::BeginTabBar(""))
            {
                if (ImGui::BeginTabItem("General Stats"))
                {
                    ImGui::Text("Name: %s", hackVariables.cPlayerName);
                    ImGui::Text("Health: %d", hackVariables.iPlayerHealth);
                    ImGui::Text("Points: %d", hackVariables.iPoints);
                    ImGui::Text("Kills: %d", hackVariables.iKills);
                    ImGui::Text("Headshots: %d", hackVariables.iHeadshots);
                    
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Toggles"))
                {
                    ImGui::Checkbox("Godmode", &hackVariables.bGodmodeToggle);
                    ImGui::Checkbox("Infinite Ammo", &hackVariables.bInfiniteAmmoToggle);
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Points"))
                {
                    ImGui::Text("Points: %d", hackVariables.iPoints);
                    ImGui::SliderInt("\t", &sliderValue, 0, 999999999);
                    if(ImGui::Button("Set Points"))
                    {
                        SetPlayerPoints(sliderValue);
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Name Changer"))
                {
                    ImGui::Text("Player Name: %s", hackVariables.cPlayerName);
                    ImGui::InputText("\t", hackVariables.cSpoofedPlayerName, sizeof(hackVariables.cSpoofedPlayerName));
                    
                    if(ImGui::Button("Spoof Name"))
                    {
                        // Set our player name as the spoofed name.
                        if (strlen(hackVariables.cSpoofedPlayerName) > 0)
                            ChangePlayerName();
                    }
                    ImGui::EndTabItem();
                }
            }
                ImGui::EndTabBar();

            if (ImGui::CollapsingHeader("Debug Info"))
            {
                ImGui::Text("Process ID: %d", hackVariables.processID);
                ImGui::Text("Player Health Addr: %lX", hackVariables.playerHealthAddress);
                ImGui::Text("Player Points Addr: %lX", hackVariables.playerPointsAddress);
                ImGui::Text("Player Kills Addr: %lX", hackVariables.playerKillsAddress);
                ImGui::Text("Player Headshots Addr: %lX", hackVariables.playerHeadshotsAddress);
                ImGui::Text("Player Grenades Addr: %lX", hackVariables.playerWeaponGrenadesAddress);
            }

        }ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    hackVariables.hackShouldStop = true;
    hackThread.join();
    return 0;
}
