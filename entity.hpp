#pragma once
#include "memory.hpp"
#include "offsets.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

struct Vector3 {
    float x = 0.f, y = 0.f, z = 0.f;

    Vector3() = default;
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator-(const Vector3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vector3 operator+(const Vector3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vector3 operator*(float s) const { return { x * s, y * s, z * s }; }

    float Length() const { return sqrtf(x * x + y * y + z * z); }
    float Length2D() const { return sqrtf(x * x + y * y); }
    float Dot(const Vector3& o) const { return x * o.x + y * o.y + z * o.z; }

    Vector3 Normalized() const {
        float len = Length();
        if (len < 0.001f) return {};
        return { x / len, y / len, z / len };
    }
};

struct Vector2 {
    float x = 0.f, y = 0.f;
};

struct Player {
    uintptr_t controller = 0;
    uintptr_t pawn = 0;
    Vector3 origin{};
    Vector3 head{};
    Vector3 velocity{};
    int health = 0;
    int armor = 0;
    int team = 0;
    bool alive = false;
    bool dormant = false;
    bool spotted = false;
    bool isTeammate = false;
    bool hasHelmet = false;
    bool hasDefuser = false;
    bool isScoped = false;
    bool isDefusing = false;
    std::string name;
    float distance = 0.f;
    int shotsFired = 0;
};

class EntitySystem {
public:
    std::vector<Player> players;
    uintptr_t localPawn = 0;
    uintptr_t localController = 0;
    int localTeam = 0;
    Vector3 localOrigin{};
    Vector3 localVelocity{};
    int localHealth = 0;
    bool localAlive = false;
    int localIndex = -1;

    void Update() {
        players.clear();
        localIndex = -1;

        localController = mem.Read<uintptr_t>(mem.clientBase + offsets::dwLocalPlayerController);
        if (!localController) return;

        localPawn = mem.Read<uintptr_t>(mem.clientBase + offsets::dwLocalPlayerPawn);
        if (!localPawn) return;

        localTeam = mem.Read<uint8_t>(localPawn + offsets::pawn::m_iTeamNum);
        localHealth = mem.Read<int>(localPawn + offsets::pawn::m_iHealth);
        localAlive = localHealth > 0 && mem.Read<uint8_t>(localPawn + offsets::pawn::m_lifeState) == 0;
        localOrigin = GetOrigin(localPawn);
        localVelocity = mem.Read<Vector3>(localPawn + offsets::pawn::m_vecAbsVelocity);

        uintptr_t entityList = mem.Read<uintptr_t>(mem.clientBase + offsets::dwEntityList);
        if (!entityList) return;

        // Find local player slot index
        for (int i = 1; i <= 64; ++i) {
            uintptr_t listEntry = mem.Read<uintptr_t>(entityList + (8 * ((i & 0x7FFF) >> 9)) + 16);
            if (!listEntry) continue;
            uintptr_t controller = mem.Read<uintptr_t>(listEntry + 112 * (i & 0x1FF));
            if (controller == localController) {
                localIndex = i - 1;
                break;
            }
        }

        for (int i = 1; i <= 64; ++i) {
            uintptr_t listEntry = mem.Read<uintptr_t>(entityList + (8 * ((i & 0x7FFF) >> 9)) + 16);
            if (!listEntry) continue;

            uintptr_t controller = mem.Read<uintptr_t>(listEntry + 112 * (i & 0x1FF));
            if (!controller || controller == localController) continue;

            uint32_t pawnHandle = mem.Read<uint32_t>(controller + offsets::controller::m_hPlayerPawn);
            if (!pawnHandle || pawnHandle == 0xFFFFFFFF) continue;

            uintptr_t pawn = mem.ResolveHandle(entityList, pawnHandle);
            if (!pawn || pawn == localPawn) continue;

            Player p{};
            p.controller = controller;
            p.pawn = pawn;

            p.health = mem.Read<int>(pawn + offsets::pawn::m_iHealth);
            p.team = mem.Read<uint8_t>(pawn + offsets::pawn::m_iTeamNum);
            uint8_t lifeState = mem.Read<uint8_t>(pawn + offsets::pawn::m_lifeState);
            p.alive = (p.health > 0 && lifeState == 0);

            // Dormant check
            uintptr_t sceneNode = mem.Read<uintptr_t>(pawn + offsets::pawn::m_pGameSceneNode);
            if (sceneNode) {
                p.dormant = mem.Read<bool>(sceneNode + offsets::scene::m_bDormant);
                p.origin = mem.Read<Vector3>(sceneNode + offsets::scene::m_vecAbsOrigin);
            } else {
                p.origin = mem.Read<Vector3>(pawn + offsets::pawn::m_vOldOrigin);
            }

            Vector3 headBone = GetBonePos(pawn, offsets::bones::head);
            if (headBone.Length() > 1.0f) {
                p.head = headBone;
            } else {
                p.head = p.origin;
                p.head.z += 72.0f;
            }

            p.velocity = mem.Read<Vector3>(pawn + offsets::pawn::m_vecAbsVelocity);
            p.armor = mem.Read<int>(pawn + offsets::pawn::m_ArmorValue);
            p.hasHelmet = mem.Read<bool>(controller + offsets::controller::m_bPawnHasHelmet);
            p.hasDefuser = mem.Read<bool>(controller + offsets::controller::m_bPawnHasDefuser);
            p.isScoped = mem.Read<bool>(pawn + offsets::pawn::m_bIsScoped);
            p.isDefusing = mem.Read<bool>(pawn + offsets::pawn::m_bIsDefusing);
            p.shotsFired = mem.Read<int>(pawn + offsets::pawn::m_iShotsFired);

            p.distance = (p.origin - localOrigin).Length() * 0.01905f;
            p.name = mem.ReadString(controller + offsets::controller::m_iszPlayerName, 32);
            p.isTeammate = (localTeam > 1 && p.team == localTeam);

            // Spotted check
            bool isSpotted = false;
            uint32_t spottedMask[2] = { 0, 0 };
            mem.ReadRaw(pawn + offsets::pawn::m_entitySpottedState + offsets::spotted::m_bSpottedByMask, spottedMask, sizeof(spottedMask));

            if (localIndex >= 0 && localIndex < 64) {
                int maskIdx = localIndex / 32;
                int bitIdx = localIndex % 32;
                bool spottedByLocal = (spottedMask[maskIdx] & (1U << bitIdx)) != 0;
                int slot1 = localIndex + 1;
                bool spottedByLocal1 = (slot1 < 64) ? ((spottedMask[slot1 / 32] & (1U << (slot1 % 32))) != 0) : false;
                isSpotted = (spottedByLocal || spottedByLocal1);
            } else {
                isSpotted = mem.Read<bool>(pawn + offsets::pawn::m_entitySpottedState + offsets::spotted::m_bSpotted);
            }
            p.spotted = isSpotted;

            if (p.alive) {
                players.push_back(p);
            }
        }
    }

    Vector3 GetOrigin(uintptr_t pawn) {
        if (!pawn) return {};
        uintptr_t sceneNode = mem.Read<uintptr_t>(pawn + offsets::pawn::m_pGameSceneNode);
        if (sceneNode)
            return mem.Read<Vector3>(sceneNode + offsets::scene::m_vecAbsOrigin);
        return mem.Read<Vector3>(pawn + offsets::pawn::m_vOldOrigin);
    }

    // Bone position (world space - CS2 bone array: 32 bytes stride)
    Vector3 GetBonePos(uintptr_t pawn, int boneIndex) {
        if (!pawn) return {};

        uintptr_t sceneNode = mem.Read<uintptr_t>(pawn + offsets::pawn::m_pGameSceneNode);
        if (!sceneNode) return {};

        uintptr_t boneArray = mem.Read<uintptr_t>(sceneNode + offsets::model::m_modelState + offsets::model::boneArray);
        if (!boneArray) return {};

        return mem.Read<Vector3>(boneArray + static_cast<uintptr_t>(boneIndex) * 32);
    }
};

inline EntitySystem entities;
