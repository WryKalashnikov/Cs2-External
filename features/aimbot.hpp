#pragma once
#include "../entity.hpp"
#include "../offsets.hpp"
#include <Windows.h>
#include <cmath>
#include <algorithm>
#include <cstdio>

namespace features {
    struct AimConfig {
        bool enabled = true;
        bool visibleCheck = false;
        float fov = 6.5f;
        float smooth = 3.8f;
        int bone = offsets::bones::head;
        int key = VK_LBUTTON;
        bool rcs = true;
        float rcsStrength = 0.65f;
    };

    inline AimConfig aimCfg;

    inline const char* GetKeyName(int vk) {
        switch (vk) {
        case 0: return "None";
        case VK_LBUTTON: return "Mouse 1";
        case VK_RBUTTON: return "Mouse 2";
        case VK_MBUTTON: return "Mouse 3";
        case VK_XBUTTON1: return "Mouse 4";
        case VK_XBUTTON2: return "Mouse 5";
        case VK_SHIFT: return "Shift";
        case VK_CONTROL: return "Ctrl";
        case VK_MENU: return "Alt";
        case VK_CAPITAL: return "Caps Lock";
        case VK_SPACE: return "Space";
        case VK_TAB: return "Tab";
        case VK_RETURN: return "Enter";
        case VK_BACK: return "Backspace";
        case VK_ESCAPE: return "Esc";
        default: {
            static char name[32];
            UINT scanCode = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
            if (scanCode != 0) {
                int result = GetKeyNameTextA(scanCode << 16, name, sizeof(name));
                if (result > 0) return name;
            }
            if (vk >= '0' && vk <= '9') {
                snprintf(name, sizeof(name), "%c", (char)vk);
                return name;
            }
            if (vk >= 'A' && vk <= 'Z') {
                snprintf(name, sizeof(name), "%c", (char)vk);
                return name;
            }
            snprintf(name, sizeof(name), "Key %d", vk);
            return name;
        }
        }
    }

    inline void RunAimbot(int width, int height, const float* matrix) {
        if (!aimCfg.enabled) return;
        if (aimCfg.key == 0 || !(GetAsyncKeyState(aimCfg.key) & 0x8000)) return;
        if (!entities.localAlive) return;

        float bestFov = aimCfg.fov;
        Vector2 targetScreen{};
        bool found = false;
        Vector3 targetWorld{};

        for (const auto& p : entities.players) {
            if (p.isTeammate || p.dormant) continue;
            if (aimCfg.visibleCheck && !p.spotted) continue;
            Vector2 screen;
            if (!WorldToScreen(p.head, screen, matrix, width, height)) continue;

            float dx = screen.x - (width * 0.5f);
            float dy = screen.y - (height * 0.5f);
            float fov = sqrtf(dx * dx + dy * dy);

            if (fov < bestFov * (height / 90.f)) {
                bestFov = fov / (height / 90.f);
                targetScreen = screen;
                targetWorld = p.head;
                found = true;
            }
        }

        if (!found) return;

        float dx = targetScreen.x - (width * 0.5f);
        float dy = targetScreen.y - (height * 0.5f);

        dx /= aimCfg.smooth;
        dy /= aimCfg.smooth;

        if (aimCfg.rcs && entities.localPawn) {
            Vector3 punch = mem.Read<Vector3>(entities.localPawn + offsets::pawn::m_aimPunchAngle);
            dy += punch.x * 2.0f * aimCfg.rcsStrength * (height / 90.f);
            dx -= punch.y * 2.0f * aimCfg.rcsStrength * (width / 90.f);
        }

        if (fabsf(dx) > 0.5f || fabsf(dy) > 0.5f) {
            mouse_event(MOUSEEVENTF_MOVE,
                static_cast<DWORD>(dx),
                static_cast<DWORD>(dy),
                0, 0);
        }
    }
}
