
#include <iostream>
#include <mutex>
#include <queue>
#include <chrono>

#include "hack.hpp"
#include "../mem/mem.hpp"

HackVariables hackVariables;

HackVariables::HackVariables(){
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

    bool hackShouldStop = false;

    std::vector<uint8_t> ammoHandlerOriginalOpcodes = {0x89, 0x84, 0x8F, 0xFC, 0x05, 0x00, 0x00};
}

void HandleGodmode(bool cleanup = false) {

    if (!cleanup) {
        if (hackVariables.iPlayerMaxHealth == hackVariables.iPlayerMaxHealthDefaultValue)
        {
            // Write player max health to a large number, when godmoded, to act as a buffer for damage.
            // Otherwise explosions, higher ranked zombies and zombie swarms could still kill the player.
            mem::WriteProcessMemory(hackVariables.processID, (void*)hackVariables.playerMaxHealthAddress, &hackVariables.iGodmodeMaxHealth, sizeof(int));

            mem::ReadProcessMemory(hackVariables.processID, (void*)hackVariables.playerMaxHealthAddress, &hackVariables.iPlayerMaxHealth, sizeof(int));
        }
        if (hackVariables.iPlayerHealth < hackVariables.iGodmodeMaxHealth)
        {
            // Write health to max health
            mem::WriteProcessMemory(hackVariables.processID, (void*)hackVariables.playerHealthAddress, &hackVariables.iGodmodeMaxHealth, sizeof(int));
        }

    } 
    else 
    {
            // Reset max health to normal, if Godmode toggle is disabled and we haven't done it already
            mem::WriteProcessMemory(hackVariables.processID, (void*)hackVariables.playerMaxHealthAddress, &hackVariables.iPlayerMaxHealthDefaultValue, sizeof(int));

            // Reset Player Health
            mem::WriteProcessMemory(hackVariables.processID, (void*)hackVariables.playerHealthAddress, &hackVariables.iPlayerMaxHealthDefaultValue, sizeof(int));

            mem::ReadProcessMemory(hackVariables.processID, (void*)hackVariables.playerMaxHealthAddress, &hackVariables.iPlayerMaxHealth, sizeof(int));
    }
}

void ToggleInfiniteAmmo(bool enabled){
    if (enabled){

        // Write NOP over assembly opcodes
        if (!mem::ReplaceBytes(hackVariables.processID, hackVariables.ammoHandlerAddress, {0x90,0x90,0x90,0x90,0x90,0x90,0x90}))
        {
            printf("replaceBytes: Couldn't replace bytes.");
        } 
        else 
        {
            hackVariables.bInfAmmoDidOverwrite = true;
        }
    } else {

        // Write the original assembly opcodes
        if (!mem::ReplaceBytes(hackVariables.processID, hackVariables.ammoHandlerAddress, hackVariables.ammoHandlerOriginalOpcodes))
        {
            printf("replaceBytes: Couldn't replace bytes.");
        }
        else
        {
            hackVariables.bInfAmmoDidOverwrite = false;
        }
    }
}

void ChangePlayerName(){

    // Write the player name.
    mem::WriteProcessMemory(hackVariables.processID, (void*)hackVariables.playerNameAddress, &hackVariables.cSpoofedPlayerName, sizeof(hackVariables.cSpoofedPlayerName));

    // Read the new player name into memory
    mem::ReadProcessMemory(hackVariables.processID, (void*)hackVariables.playerNameAddress, &hackVariables.cPlayerName, sizeof(hackVariables.cPlayerName));
}

void SetPlayerPoints(int points){
    mem::WriteProcessMemory(hackVariables.processID, (void*)hackVariables.playerPointsAddress, &points, sizeof(points));
}

void HackThread(){
    while (!hackVariables.hackShouldStop) {

        // Read needed memory each loop
        if (!hackVariables.bReadDoOnce){
            mem::ReadProcessMemory(hackVariables.processID, (void*)hackVariables.playerMaxHealthAddress, &hackVariables.iPlayerMaxHealth, sizeof(int));
            mem::ReadProcessMemory(hackVariables.processID, (void*)hackVariables.playerNameAddress, &hackVariables.cPlayerName, sizeof(hackVariables.cPlayerName));
            hackVariables.bReadDoOnce = true;
        }

        mem::ReadProcessMemory(hackVariables.processID, (void*)hackVariables.playerHealthAddress, &hackVariables.iPlayerHealth, sizeof(int));
        mem::ReadProcessMemory(hackVariables.processID, (void*)hackVariables.playerHeadshotsAddress, &hackVariables.iHeadshots, sizeof(int));
        mem::ReadProcessMemory(hackVariables.processID, (void*)hackVariables.playerKillsAddress, &hackVariables.iKills, sizeof(int));
        mem::ReadProcessMemory(hackVariables.processID, (void*)hackVariables.playerPointsAddress, &hackVariables.iPoints, sizeof(int));
        mem::ReadProcessMemory(hackVariables.processID, (void*)hackVariables.playerWeaponGrenadesAddress, &hackVariables.iGrenades, sizeof(int));

        // Godmode Toggle //
        if (hackVariables.bGodmodeToggle)
        {
            HandleGodmode();
        } 
        else if (!hackVariables.bGodmodeToggle && hackVariables.iPlayerMaxHealth == hackVariables.iGodmodeMaxHealth)
        {
            // Cleanup Godmode, resetting health/max health to what it should be.
            HandleGodmode(true);
        }

        if (hackVariables.bInfiniteAmmoToggle && !hackVariables.bInfAmmoDidOverwrite)
        {
            ToggleInfiniteAmmo(true);

        } 
        else if (!hackVariables.bInfiniteAmmoToggle && hackVariables.bInfAmmoDidOverwrite)
        {
            ToggleInfiniteAmmo(false);
        }
    }
}