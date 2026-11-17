#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>
#include <string>
#include <thread>
#include <chrono>
#include <cmath>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

#include "memory.hpp"
#include "offsets.hpp"
#include "entity.hpp"
#include "features/esp.hpp"
#include "features/aimbot.hpp"
#include "features/misc.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

// ==================== D3D11 ====================
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;
static HWND                     g_hwnd = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ==================== MENU STATE ====================
struct MenuState {
    bool open = true;
    int currentTab = 0;
} menu;

// ==================== RENDER MENU ====================
void RenderMenu() {
    if (!menu.open) return;

    ImGui::SetNextWindowSize(ImVec2(760, 560), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(680, 500), ImVec2(1100, 760));
    ImGui::SetNextWindowBgAlpha(0.98f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
    if (!ImGui::Begin("NEXUS  //  CONTROL CENTER##MainWindow", &menu.open, flags)) {
        ImGui::End();
        return;
    }

    // Left navigation / status rail
    ImGui::BeginChild("##sidebar", ImVec2(176, 0), true, ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::TextColored(ImVec4(0.31f, 0.92f, 0.62f, 1.0f), "NEXUS");
        ImGui::TextDisabled("EXTERNAL CONTROL");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        const char* tabs[] = { "  VISUALS", "  AIM CONTROL", "  UTILITIES", "  PALETTE", "  SYSTEM" };
        for (int i = 0; i < 5; ++i) {
            bool selected = (menu.currentTab == i);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.55f, 0.32f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.62f, 0.38f, 1.0f));
            }
            if (ImGui::Button(tabs[i], ImVec2(-1, 40)))
                menu.currentTab = i;
            if (selected) ImGui::PopStyleColor(2);
            ImGui::Spacing();
        }
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 72.0f);
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.30f, 0.88f, 0.58f, 1.0f), "●  ONLINE");
        ImGui::TextDisabled("INSERT  to toggle");
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##content", ImVec2(0, 0), true);
    {
        ImGui::TextDisabled("CONFIGURATION / LIVE PROFILE");
        ImGui::SameLine(ImGui::GetWindowWidth() - 132.0f);
        ImGui::TextColored(ImVec4(0.30f, 0.88f, 0.58f, 1.0f), "● ACTIVE");
        ImGui::Separator();
        ImGui::Spacing();
        if (menu.currentTab == 0) { // Visuals
            ImGui::TextUnformatted("VISUALS");
            ImGui::TextDisabled("Configure the information rendered around entities.");
            ImGui::Separator();
            ImGui::Checkbox("Enable ESP", &features::espCfg.enabled);
            ImGui::Checkbox("Team Check (Enemies Only)", &features::espCfg.teamCheck);
            ImGui::Checkbox("Box", &features::espCfg.box);
            ImGui::Checkbox("Filled Box", &features::espCfg.filledBox);
            ImGui::Checkbox("Health Bar", &features::espCfg.healthBar);
            ImGui::Checkbox("Armor Bar", &features::espCfg.armorBar);
            ImGui::Checkbox("Name", &features::espCfg.name);
            ImGui::Checkbox("Distance", &features::espCfg.distance);
            ImGui::Checkbox("Head Dot", &features::espCfg.headDot);
            ImGui::Checkbox("Snaplines", &features::espCfg.snaplines);
            ImGui::Checkbox("Skeleton", &features::espCfg.skeleton);
            ImGui::SliderFloat("Box Thickness", &features::espCfg.boxThickness, 0.8f, 3.0f);
        }
        else if (menu.currentTab == 1) { // Aimbot
            ImGui::TextUnformatted("AIM CONTROL");
            ImGui::TextDisabled("Precision and input behavior.");
            ImGui::Separator();
            ImGui::Checkbox("Enable Aimbot", &features::aimCfg.enabled);
            ImGui::Checkbox("Visible Check (Only Visible)", &features::aimCfg.visibleCheck);
            ImGui::SliderFloat("FOV", &features::aimCfg.fov, 1.0f, 20.0f, "%.1f");
            ImGui::SliderFloat("Smooth", &features::aimCfg.smooth, 1.0f, 12.0f, "%.1f");
            ImGui::Checkbox("Recoil Control", &features::aimCfg.rcs);
            if (features::aimCfg.rcs)
                ImGui::SliderFloat("RCS Strength", &features::aimCfg.rcsStrength, 0.1f, 1.0f, "%.2f");
            
            ImGui::Spacing();
            ImGui::Text("Aim Key");

            static bool isBindingAimKey = false;
            static bool waitingForRelease = false;

            char btnLabel[64];
            if (isBindingAimKey) {
                snprintf(btnLabel, sizeof(btnLabel), "[ Press Key/Mouse... ]##AimKey");
            } else {
                snprintf(btnLabel, sizeof(btnLabel), "[ %s ]##AimKey", features::GetKeyName(features::aimCfg.key));
            }

            if (ImGui::Button(btnLabel, ImVec2(160, 24))) {
                isBindingAimKey = true;
                waitingForRelease = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Unbind##AimKey", ImVec2(70, 24))) {
                features::aimCfg.key = 0;
                isBindingAimKey = false;
                waitingForRelease = false;
            }

            if (isBindingAimKey) {
                ImGui::TextDisabled("Press any key or mouse button (Esc to cancel)");
                if (waitingForRelease) {
                    if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
                        waitingForRelease = false;
                    }
                } else {
                    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                        isBindingAimKey = false;
                    } else {
                        const int mouseButtons[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
                        for (int k : mouseButtons) {
                            if (GetAsyncKeyState(k) & 0x8000) {
                                features::aimCfg.key = k;
                                isBindingAimKey = false;
                                break;
                            }
                        }
                        if (isBindingAimKey) {
                            for (int k = 0x07; k <= 0xFE; ++k) {
                                if (k == VK_LBUTTON || k == VK_RBUTTON || k == VK_MBUTTON || k == VK_XBUTTON1 || k == VK_XBUTTON2 || k == VK_ESCAPE)
                                    continue;
                                if (GetAsyncKeyState(k) & 0x8000) {
                                    features::aimCfg.key = k;
                                    isBindingAimKey = false;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (menu.currentTab == 2) { // Misc
            ImGui::TextUnformatted("UTILITIES");
            ImGui::TextDisabled("Movement, protection and trigger options.");
            ImGui::Separator();
            ImGui::Checkbox("Bunnyhop", &features::miscCfg.bhop);
            ImGui::Checkbox("No Flash", &features::miscCfg.noFlash);
            if (features::miscCfg.noFlash)
                ImGui::SliderFloat("Flash Alpha", &features::miscCfg.noFlashAlpha, 0.0f, 1.0f);

            ImGui::Spacing();
            ImGui::Checkbox("Triggerbot", &features::miscCfg.triggerbot);
            if (features::miscCfg.triggerbot) {
                ImGui::Checkbox("Always On", &features::miscCfg.triggerAlwaysOn);
                ImGui::SliderInt("Trigger Delay (ms)", &features::miscCfg.triggerDelay, 0, 150);

                if (!features::miscCfg.triggerAlwaysOn) {
                    ImGui::Spacing();
                    ImGui::Text("Trigger Key");

                    static bool isBindingTriggerKey = false;
                    static bool waitingForTriggerRelease = false;

                    char triggerBtnLabel[64];
                    if (isBindingTriggerKey) {
                        snprintf(triggerBtnLabel, sizeof(triggerBtnLabel), "[ Press Key/Mouse... ]##TriggerKey");
                    } else {
                        snprintf(triggerBtnLabel, sizeof(triggerBtnLabel), "[ %s ]##TriggerKey", features::GetKeyName(features::miscCfg.triggerKey));
                    }

                    if (ImGui::Button(triggerBtnLabel, ImVec2(160, 24))) {
                        isBindingTriggerKey = true;
                        waitingForTriggerRelease = true;
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Unbind##TriggerKey", ImVec2(70, 24))) {
                        features::miscCfg.triggerKey = 0;
                        isBindingTriggerKey = false;
                        waitingForTriggerRelease = false;
                    }

                    if (isBindingTriggerKey) {
                        ImGui::TextDisabled("Press any key or mouse button (Esc to cancel)");
                        if (waitingForTriggerRelease) {
                            if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
                                waitingForTriggerRelease = false;
                            }
                        } else {
                            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                                isBindingTriggerKey = false;
                            } else {
                                const int mouseButtons[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
                                for (int k : mouseButtons) {
                                    if (GetAsyncKeyState(k) & 0x8000) {
                                        features::miscCfg.triggerKey = k;
                                        isBindingTriggerKey = false;
                                        break;
                                    }
                                }
                                if (isBindingTriggerKey) {
                                    for (int k = 0x07; k <= 0xFE; ++k) {
                                        if (k == VK_LBUTTON || k == VK_RBUTTON || k == VK_MBUTTON || k == VK_XBUTTON1 || k == VK_XBUTTON2 || k == VK_ESCAPE)
                                            continue;
                                        if (GetAsyncKeyState(k) & 0x8000) {
                                            features::miscCfg.triggerKey = k;
                                            isBindingTriggerKey = false;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    ImGui::TextDisabled("Always-on mode: automatically fires when target is in crosshair");
                }
            }
        }
        else if (menu.currentTab == 3) { // Colors
            ImGui::TextUnformatted("PALETTE");
            ImGui::TextDisabled("Tune the overlay appearance without changing behavior.");
            ImGui::Separator();
            ImGui::ColorEdit4("Box Color", features::espCfg.boxColor, ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("Name Color", features::espCfg.nameColor, ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit4("Snapline Color", features::espCfg.snaplineColor, ImGuiColorEditFlags_NoInputs);
        }
        else if (menu.currentTab == 4) { // Info
            ImGui::TextUnformatted("SYSTEM");
            ImGui::TextDisabled("Runtime and build information.");
            ImGui::Separator();
            ImGui::Text("Build: %s %s", __DATE__, __TIME__);
            ImGui::Text("By Wry");
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

// ==================== MAIN ====================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // Attach to CS2
    if (!mem.Attach(L"cs2.exe")) {
        MessageBoxA(nullptr, "CS2 process not found.\nOpen the game first.", "CS2 External", MB_ICONERROR);
        return 1;
    }

    mem.clientBase = mem.GetModuleBase(L"client.dll");
    mem.engineBase = mem.GetModuleBase(L"engine2.dll");

    if (!mem.clientBase) {
        MessageBoxA(nullptr, "client.dll base not found.", "CS2 External", MB_ICONERROR);
        return 1;
    }

    // Transparent overlay window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
        GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        L"CS2ExternalClass", nullptr };
    RegisterClassExW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName, L" ",
        WS_POPUP, 0, 0, screenW, screenH,
        nullptr, nullptr, wc.hInstance, nullptr);

    SetLayeredWindowAttributes(g_hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(g_hwnd, &margin);

    if (!CreateDeviceD3D(g_hwnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    // UI theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 14.f;
    style.ChildRounding = 10.f;
    style.FrameRounding = 7.f;
    style.PopupRounding = 8.f;
    style.ScrollbarRounding = 8.f;
    style.GrabRounding = 7.f;
    style.TabRounding = 7.f;
    style.WindowBorderSize = 1.f;
    style.FrameBorderSize = 1.f;
    style.ItemSpacing = ImVec2(10, 8);
    style.WindowPadding = ImVec2(16, 16);

    // Deep graphite surface with a single emerald accent.
    style.Colors[ImGuiCol_WindowBg]         = ImVec4(0.035f, 0.045f, 0.060f, 0.985f);
    style.Colors[ImGuiCol_ChildBg]          = ImVec4(0.050f, 0.065f, 0.082f, 1.00f);
    style.Colors[ImGuiCol_Border]           = ImVec4(0.12f, 0.18f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.075f, 0.095f, 0.115f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.10f, 0.14f, 0.16f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive]    = ImVec4(0.12f, 0.17f, 0.18f, 1.00f);
    style.Colors[ImGuiCol_TitleBg]          = ImVec4(0.025f, 0.035f, 0.045f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive]    = ImVec4(0.035f, 0.060f, 0.065f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]        = ImVec4(0.25f, 0.92f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]       = ImVec4(0.20f, 0.70f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.95f, 0.65f, 1.00f);
    style.Colors[ImGuiCol_Button]           = ImVec4(0.08f, 0.20f, 0.18f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered]    = ImVec4(0.12f, 0.34f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]     = ImVec4(0.16f, 0.45f, 0.34f, 1.00f);
    style.Colors[ImGuiCol_Header]           = ImVec4(0.08f, 0.26f, 0.21f, 0.85f);
    style.Colors[ImGuiCol_HeaderHovered]    = ImVec4(0.12f, 0.38f, 0.29f, 0.95f);
    style.Colors[ImGuiCol_HeaderActive]     = ImVec4(0.16f, 0.50f, 0.38f, 1.00f);

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool running = true;
    while (running) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) running = false;
        }
        if (!running) break;

        // Toggle menu
        if (GetAsyncKeyState(VK_INSERT) & 1)
            menu.open = !menu.open;

        // Toggle click-through when menu closed
        LONG_PTR exStyle = GetWindowLongPtr(g_hwnd, GWL_EXSTYLE);
        if (menu.open) {
            SetWindowLongPtr(g_hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
        } else {
            SetWindowLongPtr(g_hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
        }

        // Update entities
        entities.Update();

        // View matrix
        float matrix[16]{};
        mem.ReadRaw(mem.clientBase + offsets::dwViewMatrix, matrix, sizeof(matrix));

        // New frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Features
        features::RenderESP(screenW, screenH, matrix);
        features::RunAimbot(screenW, screenH, matrix);
        features::RunMisc();

        // Menu
        RenderMenu();

        // Render
        ImGui::Render();
        const float clear_color[4] = { 0.f, 0.f, 0.f, 0.f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(0, 0); // no vsync

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(g_hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    mem.Close();
    return 0;
}

// ==================== D3D HELPERS ====================
bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
        &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);

    if (res != S_OK) return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

// Win32 message handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
