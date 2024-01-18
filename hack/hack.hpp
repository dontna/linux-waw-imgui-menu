#pragma once

#include <functional>
#include <thread>

struct HackVariables {

    pid_t processID = 0;
    const uintptr_t playerHealthAddress = 0x0176C8B8;
    const uintptr_t playerMaxHealthAddress = 0x0176C8BC;
    const uintptr_t playerPointsAddress = 0x018EF124;
    const uintptr_t playerKillsAddress = 0x018EF128;
    const uintptr_t playerHeadshotsAddress = 0x018EF138;
    const uintptr_t playerNameAddress = 0x018EF258;
    const uintptr_t playerWeaponGrenadesAddress = 0x018ED674;

    uintptr_t ammoHandlerAddress = 0x0041E619;
    
    const char* processName = "CoDWaW.exe";
    char cPlayerName[32] = "";
    char cSpoofedPlayerName[32] = "";

    int iPlayerHealth = 0;
    int iPlayerMaxHealth = 0;
    int iPlayerMaxHealthDefaultValue = 100;
    
    int iGodmodeMaxHealth = 10000;

    int iGrenades = 0;

    int iPoints = 0;
    int iKills = 0;
    int iHeadshots = 0;

    bool bGodmodeToggle = false;
    bool bInfiniteAmmoToggle = false;
    bool bInfAmmoDidOverwrite = false;

    bool bReadDoOnce = false; // Used to read values, that aren't consistantly changing; such as Player Name.

    bool bReadName = false; // Used if the name is edited, so we can read the new name.
    bool bReadMaxHealth = false; // Used if the max health is editied, so we can read the new health

    bool hackShouldStop = false;

    std::vector<uint8_t> ammoHandlerOriginalOpcodes = {0x89, 0x84, 0x8F, 0xFC, 0x05, 0x00, 0x00};

    HackVariables();
};

extern HackVariables hackVariables; // Delclare the global variable.

void HackThread();
void ChangePlayerName();
void SetPlayerPoints(int points);