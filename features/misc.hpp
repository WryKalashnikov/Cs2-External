#pragma once
#include "../entity.hpp"
#include "../offsets.hpp"
#include <Windows.h>
#include <thread>
#include <chrono>

namespace features {
    struct MiscConfig {
        bool bhop = true;
        bool radarHack = false;
        bool noFlash = false;
        float noFlashAlpha = 0.0f;
        bool triggerbot = false;
        bool triggerAlwaysOn = false;
        int triggerKey = VK_XBUTTON2;
        int triggerDelay = 20;
    };

    inline MiscConfig miscCfg;

    inline void RunBhop() {
        if (!miscCfg.bhop) return;
        if (!entities.localAlive || !entities.localPawn) return;

        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            int flags = mem.Read<int>(entities.localPawn + offsets::pawn::m_fFlags);
            if (flags & 1) {
                // FL_ONGROUND
            }
        }
    }

    inline void RunNoFlash() {
        if (!miscCfg.noFlash || !entities.localPawn) return;
        float duration = mem.Read<float>(entities.localPawn + offsets::pawn::m_flFlashDuration);
        if (duration > 0.0f) {
            mem.Write<float>(entities.localPawn + offsets::pawn::m_flFlashDuration, miscCfg.noFlashAlpha);
            mem.Write<float>(entities.localPawn + offsets::pawn::m_flFlashMaxAlpha, miscCfg.noFlashAlpha);
        }
    }

    inline void RunTriggerbot() {
        if (!miscCfg.triggerbot) return;
        if (!entities.localAlive || !entities.localPawn) return;

        if (!miscCfg.triggerAlwaysOn) {
            if (miscCfg.triggerKey == 0 || !(GetAsyncKeyState(miscCfg.triggerKey) & 0x8000)) return;
        }

        static auto lastShotTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastShotTime).count() < 120) {
            return;
        }

        int crosshairId = mem.Read<int>(entities.localPawn + offsets::pawn::m_iIDEntIndex);
        if (crosshairId <= 0) return;

        uintptr_t entityList = mem.Read<uintptr_t>(mem.clientBase + offsets::dwEntityList);
        if (!entityList) return;

        uintptr_t listEntry = mem.Read<uintptr_t>(entityList + (8 * ((crosshairId & 0x7FFF) >> 9)) + 16);
        if (!listEntry) return;

        uintptr_t targetPawn = mem.Read<uintptr_t>(listEntry + 112 * (crosshairId & 0x1FF));
        if (!targetPawn || targetPawn == entities.localPawn) return;

        int targetTeam = mem.Read<uint8_t>(targetPawn + offsets::pawn::m_iTeamNum);
        int targetHealth = mem.Read<int>(targetPawn + offsets::pawn::m_iHealth);
        uint8_t lifeState = mem.Read<uint8_t>(targetPawn + offsets::pawn::m_lifeState);

        if (targetHealth <= 0 || lifeState != 0) return;
        if (targetTeam == entities.localTeam) return;

        if (miscCfg.triggerDelay > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(miscCfg.triggerDelay));
        }

        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        lastShotTime = std::chrono::steady_clock::now();
    }

    inline void RunMisc() {
        RunBhop();
        RunNoFlash();
        RunTriggerbot();
    }
}
