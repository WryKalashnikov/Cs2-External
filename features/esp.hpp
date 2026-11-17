#pragma once
#include "../entity.hpp"
#include "../offsets.hpp"
#include <imgui.h>
#include <algorithm>
#include <utility>

namespace features {
    struct EspConfig {
        bool enabled = true;
        bool teamCheck = true;
        bool box = true;
        bool filledBox = false;
        bool healthBar = true;
        bool armorBar = false;
        bool name = true;
        bool distance = true;
        bool weapon = false;
        bool skeleton = true;
        bool headDot = true;
        bool snaplines = false;

        float boxThickness = 1.6f;
        float boxColor[4] = { 0.15f, 0.85f, 0.40f, 1.0f };
        float boxColorVisible[4] = { 0.20f, 0.95f, 0.50f, 1.0f };
        float healthColor[4] = { 0.15f, 0.90f, 0.30f, 1.0f };
        float nameColor[4] = { 0.95f, 0.95f, 0.95f, 1.0f };
        float snaplineColor[4] = { 0.8f, 0.2f, 0.2f, 0.7f };
    };

    inline EspConfig espCfg;

    inline bool WorldToScreen(const Vector3& world, Vector2& screen, const float* matrix, int width, int height) {
        float w = matrix[12] * world.x + matrix[13] * world.y + matrix[14] * world.z + matrix[15];
        if (w < 0.01f) return false;

        float invW = 1.0f / w;
        float x = (matrix[0] * world.x + matrix[1] * world.y + matrix[2] * world.z + matrix[3]) * invW;
        float y = (matrix[4] * world.x + matrix[5] * world.y + matrix[6] * world.z + matrix[7]) * invW;

        screen.x = (width * 0.5f) + (width * 0.5f) * x;
        screen.y = (height * 0.5f) - (height * 0.5f) * y;
        return true;
    }

    inline void RenderESP(int width, int height, const float* matrix) {
        if (!espCfg.enabled) return;

        ImDrawList* draw = ImGui::GetBackgroundDrawList();
        if (!draw) return;

        for (const auto& p : entities.players) {
            if (espCfg.teamCheck && p.isTeammate) continue;
            if (p.dormant && p.origin.Length() < 1.0f) continue;

            Vector2 feet, head;
            if (!WorldToScreen(p.origin, feet, matrix, width, height)) continue;
            if (!WorldToScreen(p.head, head, matrix, width, height)) continue;

            float boxH = fabsf(feet.y - head.y);
            if (boxH < 8.f) continue;
            float boxW = boxH * 0.45f;
            float boxTop = std::min(feet.y, head.y);
            float boxBottom = std::max(feet.y, head.y);
            float boxX = head.x - boxW * 0.5f;

            ImU32 boxCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
                espCfg.boxColor[0], espCfg.boxColor[1], espCfg.boxColor[2], espCfg.boxColor[3]));

            // Box
            if (espCfg.box) {
                if (espCfg.filledBox) {
                    draw->AddRectFilled(
                        ImVec2(boxX, boxTop),
                        ImVec2(boxX + boxW, boxBottom),
                        IM_COL32(
                            static_cast<int>(espCfg.boxColor[0] * 255),
                            static_cast<int>(espCfg.boxColor[1] * 255),
                            static_cast<int>(espCfg.boxColor[2] * 255),
                            40));
                }
                draw->AddRect(
                    ImVec2(boxX, boxTop),
                    ImVec2(boxX + boxW, boxBottom),
                    boxCol, 0.f, 0, espCfg.boxThickness);
            }

            // Health bar
            if (espCfg.healthBar) {
                float pct = std::clamp(p.health / 100.f, 0.f, 1.f);
                float barH = boxH * pct;
                float barX = boxX - 6.f;

                // Background
                draw->AddRectFilled(ImVec2(barX, boxTop), ImVec2(barX + 3.f, boxBottom), IM_COL32(20, 20, 20, 180));
                // Health
                ImU32 hpCol = IM_COL32(
                    static_cast<int>((1.f - pct) * 220),
                    static_cast<int>(pct * 220),
                    40, 255);
                draw->AddRectFilled(ImVec2(barX, boxBottom - barH), ImVec2(barX + 3.f, boxBottom), hpCol);
            }

            // Armor bar
            if (espCfg.armorBar && p.armor > 0) {
                float pct = std::clamp(p.armor / 100.f, 0.f, 1.f);
                float barH = boxH * pct;
                float barX = boxX - 10.f;
                draw->AddRectFilled(ImVec2(barX, boxTop), ImVec2(barX + 3.f, boxBottom), IM_COL32(20, 20, 20, 180));
                draw->AddRectFilled(ImVec2(barX, boxBottom - barH), ImVec2(barX + 3.f, boxBottom), IM_COL32(60, 140, 255, 255));
            }

            // Name + Distance
            if (espCfg.name || espCfg.distance) {
                char buf[96]{};
                if (espCfg.name && espCfg.distance)
                    snprintf(buf, sizeof(buf), "%s  %.0fm", p.name.c_str(), p.distance);
                else if (espCfg.name)
                    snprintf(buf, sizeof(buf), "%s", p.name.c_str());
                else
                    snprintf(buf, sizeof(buf), "%.0fm", p.distance);

                ImVec2 textSize = ImGui::CalcTextSize(buf);
                float textX = head.x - textSize.x * 0.5f;
                float textY = boxTop - textSize.y - 3.f;

                // Shadow
                draw->AddText(ImVec2(textX + 1, textY + 1), IM_COL32(0, 0, 0, 200), buf);
                draw->AddText(ImVec2(textX, textY),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(espCfg.nameColor[0], espCfg.nameColor[1], espCfg.nameColor[2], espCfg.nameColor[3])),
                    buf);
            }

            // Head dot
            if (espCfg.headDot) {
                draw->AddCircleFilled(ImVec2(head.x, head.y), 2.5f, IM_COL32(255, 50, 50, 220));
            }

            // Snaplines
            if (espCfg.snaplines) {
                draw->AddLine(
                    ImVec2(width * 0.5f, static_cast<float>(height)),
                    ImVec2(feet.x, feet.y),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(espCfg.snaplineColor[0], espCfg.snaplineColor[1], espCfg.snaplineColor[2], espCfg.snaplineColor[3])),
                    1.2f);
            }

            // ==================== SKELETON ====================
            if (espCfg.skeleton) {
                // Bone connections (CS2 player rig)
                static const std::pair<int, int> connections[] = {
                    // Spine
                    { offsets::bones::pelvis, offsets::bones::spine },
                    { offsets::bones::spine,  offsets::bones::neck },
                    { offsets::bones::neck,   offsets::bones::head },

                    // Left arm
                    { offsets::bones::neck,       offsets::bones::l_shoulder },
                    { offsets::bones::l_shoulder, offsets::bones::l_elbow },
                    { offsets::bones::l_elbow,    offsets::bones::l_hand },

                    // Right arm
                    { offsets::bones::neck,       offsets::bones::r_shoulder },
                    { offsets::bones::r_shoulder, offsets::bones::r_elbow },
                    { offsets::bones::r_elbow,    offsets::bones::r_hand },

                    // Left leg
                    { offsets::bones::pelvis, offsets::bones::l_hip },
                    { offsets::bones::l_hip,  offsets::bones::l_knee },
                    { offsets::bones::l_knee, offsets::bones::l_foot },

                    // Right leg
                    { offsets::bones::pelvis, offsets::bones::r_hip },
                    { offsets::bones::r_hip,  offsets::bones::r_knee },
                    { offsets::bones::r_knee, offsets::bones::r_foot },
                };

                ImU32 skelCol = IM_COL32(220, 220, 255, 230);

                for (const auto& conn : connections) {
                    Vector3 boneA = entities.GetBonePos(p.pawn, conn.first);
                    Vector3 boneB = entities.GetBonePos(p.pawn, conn.second);

                    // Skip invalid bones or disconnected points
                    if (boneA.Length() < 1.f || boneB.Length() < 1.f) continue;
                    if ((boneA - boneB).Length() > 45.0f) continue;

                    Vector2 screenA, screenB;
                    if (!WorldToScreen(boneA, screenA, matrix, width, height)) continue;
                    if (!WorldToScreen(boneB, screenB, matrix, width, height)) continue;

                    draw->AddLine(
                        ImVec2(screenA.x, screenA.y),
                        ImVec2(screenB.x, screenB.y),
                        skelCol, 1.4f);
                }

                // Head circle on skeleton
                Vector3 headBone = entities.GetBonePos(p.pawn, offsets::bones::head);
                Vector2 headScreen;
                if (WorldToScreen(headBone, headScreen, matrix, width, height)) {
                    draw->AddCircle(ImVec2(headScreen.x, headScreen.y), 3.5f, skelCol, 0, 1.5f);
                }
            }
        }
    }
}
