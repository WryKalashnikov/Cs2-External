#pragma once
#include <cstdint>

namespace offsets {
    constexpr uintptr_t dwCSGOInput                 = 0x2569E30;
    constexpr uintptr_t dwEntityList                = 0x2709010;
    constexpr uintptr_t dwGameEntitySystem          = 0x2709010;
    constexpr uintptr_t dwGameEntitySystem_highestEntityIndex = 0x2120;
    constexpr uintptr_t dwGameRules                 = 0x25537E8;
    constexpr uintptr_t dwGlobalVars                = 0x2221AB8;
    constexpr uintptr_t dwGlowManager               = 0x2550500;
    constexpr uintptr_t dwLocalPlayerController     = 0x252B0C8;
    constexpr uintptr_t dwLocalPlayerPawn           = 0x2553D38;
    constexpr uintptr_t dwPlantedC4                 = 0x24B9B90;
    constexpr uintptr_t dwPrediction                = 0x2553C40;
    constexpr uintptr_t dwViewAngles                = 0x256A4B8;
    constexpr uintptr_t dwViewMatrix                = 0x2559640;
    constexpr uintptr_t dwViewRender                = 0x255A008;
    constexpr uintptr_t dwWeaponC4                  = 0x24B83B0;

    namespace engine2 {
        constexpr uintptr_t dwBuildNumber                        = 0x6B2D18;
        constexpr uintptr_t dwNetworkGameClient                  = 0x9AFB30;
        constexpr uintptr_t dwNetworkGameClient_clientTickCount  = 0x398;
        constexpr uintptr_t dwNetworkGameClient_deltaTick        = 0x24C;
        constexpr uintptr_t dwNetworkGameClient_isBackgroundMap  = 0x2C143F;
        constexpr uintptr_t dwNetworkGameClient_localPlayer      = 0xF8;
        constexpr uintptr_t dwNetworkGameClient_maxClients       = 0x240;
        constexpr uintptr_t dwNetworkGameClient_serverTickCount  = 0x24C;
        constexpr uintptr_t dwNetworkGameClient_signOnState      = 0x230;
        constexpr uintptr_t dwWindowHeight                       = 0x9B4294;
        constexpr uintptr_t dwWindowWidth                        = 0x9B4290;
    }

    namespace buttons {
        constexpr uintptr_t attack       = 0x2225900;
        constexpr uintptr_t attack2      = 0x2225990;
        constexpr uintptr_t back         = 0x2225BD0;
        constexpr uintptr_t duck         = 0x2225EA0;
        constexpr uintptr_t forward      = 0x2225B40;
        constexpr uintptr_t jump         = 0x2225E10;
        constexpr uintptr_t left         = 0x2225C60;
        constexpr uintptr_t lookatweapon = 0x2569D50;
        constexpr uintptr_t reload       = 0x2225870;
        constexpr uintptr_t right        = 0x2225CF0;
        constexpr uintptr_t showscores   = 0x2569C30;
        constexpr uintptr_t sprint       = 0x22257E0;
        constexpr uintptr_t turnleft     = 0x2225A20;
        constexpr uintptr_t turnright    = 0x2225AB0;
        constexpr uintptr_t use          = 0x2225D80;
        constexpr uintptr_t zoom         = 0x2569CC0;
    }

    namespace controller {
        constexpr uintptr_t m_hPlayerPawn           = 0x92C;
        constexpr uintptr_t m_hObserverPawn         = 0x930;
        constexpr uintptr_t m_bPawnIsAlive          = 0x934;
        constexpr uintptr_t m_iPawnHealth           = 0x938;
        constexpr uintptr_t m_iPawnArmor            = 0x93C;
        constexpr uintptr_t m_bPawnHasDefuser       = 0x940;
        constexpr uintptr_t m_bPawnHasHelmet        = 0x941;
        constexpr uintptr_t m_nPawnCharacterDefIndex= 0x942;
        constexpr uintptr_t m_iPawnLifetimeStart     = 0x944;
        constexpr uintptr_t m_iPawnLifetimeEnd       = 0x948;
        constexpr uintptr_t m_iPawnBotDifficulty    = 0x94C;
        constexpr uintptr_t m_hOriginalControllerOfCurrentPawn = 0x950;
        constexpr uintptr_t m_iScore                = 0x954;
        constexpr uintptr_t m_iszPlayerName         = 0x6FC;
        constexpr uintptr_t m_steamID               = 0x788;
        constexpr uintptr_t m_iConnected            = 0x6F4;
    }

    namespace pawn {
        constexpr uintptr_t m_pGameSceneNode        = 0x330;
        constexpr uintptr_t m_iHealth               = 0x34C;
        constexpr uintptr_t m_lifeState             = 0x354;
        constexpr uintptr_t m_iTeamNum              = 0x3E7;
        constexpr uintptr_t m_fFlags                = 0x3F4;
        constexpr uintptr_t m_vecAbsVelocity        = 0x3F8;
        constexpr uintptr_t m_vOldOrigin            = 0x14A4;
        constexpr uintptr_t m_pClippingWeapon       = 0x13A0;
        constexpr uintptr_t m_flFlashDuration       = 0x1510;
        constexpr uintptr_t m_flFlashMaxAlpha       = 0x150C;
        constexpr uintptr_t m_ArmorValue            = 0x1F00;
        constexpr uintptr_t m_bIsScoped             = 0x1ED0;
        constexpr uintptr_t m_bIsDefusing           = 0x1ED2;
        constexpr uintptr_t m_bIsGrabbingHostage    = 0x1ED3;
        constexpr uintptr_t m_bIsBuyMenuOpen        = 0x15EA;
        constexpr uintptr_t m_flEmitSoundTime       = 0x1ED8;
        constexpr uintptr_t m_aimPunchAngle         = 0x1738;
        constexpr uintptr_t m_aimPunchAngleVel      = 0x1744;
        constexpr uintptr_t m_iShotsFired           = 0x1EE8;
        constexpr uintptr_t m_flVelocityModifier    = 0x1EF0;
        constexpr uintptr_t m_pWeaponServices        = 0x12F0;
        constexpr uintptr_t m_pItemServices          = 0x12F8;
        constexpr uintptr_t m_iIDEntIndex           = 0x370C;
        constexpr uintptr_t m_entitySpottedState    = 0x1EB8;
    }

    namespace spotted {
        constexpr uintptr_t m_bSpotted              = 0x8;
        constexpr uintptr_t m_bSpottedByMask        = 0xC;
    }

    namespace scene {
        constexpr uintptr_t m_nodeToWorld           = 0x10;
        constexpr uintptr_t m_pOwner                = 0x30;
        constexpr uintptr_t m_pParent               = 0x38;
        constexpr uintptr_t m_pChild                = 0x40;
        constexpr uintptr_t m_pNextSibling          = 0x48;
        constexpr uintptr_t m_hParent               = 0x70;
        constexpr uintptr_t m_vecOrigin             = 0x80;
        constexpr uintptr_t m_angRotation           = 0xB8;
        constexpr uintptr_t m_flScale               = 0xC4;
        constexpr uintptr_t m_vecAbsOrigin          = 0xC8;
        constexpr uintptr_t m_angAbsRotation        = 0xD4;
        constexpr uintptr_t m_flAbsScale            = 0xE0;
        constexpr uintptr_t m_nParentAttachmentOrBone = 0x100;
        constexpr uintptr_t m_bDebugAbsOriginChanges = 0x102;
        constexpr uintptr_t m_bDormant              = 0x103;
        constexpr uintptr_t m_bForceParentToBeNetworked = 0x104;
        constexpr uintptr_t m_nHierarchicalDepth    = 0x107;
        constexpr uintptr_t m_nHierarchyType        = 0x108;
        constexpr uintptr_t m_name                  = 0x10C;
    }

    namespace model {
        constexpr uintptr_t m_modelState            = 0x140;
        constexpr uintptr_t m_bIsAnimationEnabled   = 0x430;
        constexpr uintptr_t m_nHitboxSet            = 0x3FC;

        constexpr uintptr_t boneArray               = 0x80;
        constexpr uintptr_t boneStride              = 0x20;
    }

    namespace bones {
        constexpr int pelvis     = 1;
        constexpr int spine      = 4;
        constexpr int neck       = 6;
        constexpr int head       = 7;
        constexpr int l_shoulder = 8;
        constexpr int l_elbow    = 10;
        constexpr int l_hand     = 11;
        constexpr int r_shoulder = 12;
        constexpr int r_elbow    = 14;
        constexpr int r_hand     = 15;
        constexpr int l_hip      = 17;
        constexpr int l_knee     = 18;
        constexpr int l_foot     = 19;
        constexpr int r_hip      = 20;
        constexpr int r_knee     = 21;
        constexpr int r_foot     = 22;
    }

    namespace weapon {
        constexpr uintptr_t m_iItemDefinitionIndex  = 0x1BA0;
        constexpr uintptr_t m_AttributeManager      = 0x1290;
        constexpr uintptr_t m_bInReload             = 0x1A7C;
        constexpr uintptr_t m_flNextPrimaryAttack   = 0x1630;
        constexpr uintptr_t m_nNextPrimaryAttackTick = 0x1918;
        constexpr uintptr_t m_flRecoilIndex         = 0x1A68;
    }

    namespace glow {
        constexpr uintptr_t m_iGlowType             = 0x30;
        constexpr uintptr_t m_iGlowTeam             = 0x34;
        constexpr uintptr_t m_nGlowRange             = 0x38;
        constexpr uintptr_t m_nGlowRangeMin          = 0x3C;
        constexpr uintptr_t m_glowColorOverride     = 0x40;
        constexpr uintptr_t m_bFlashing             = 0x44;
        constexpr uintptr_t m_flGlowTime            = 0x48;
        constexpr uintptr_t m_flGlowStartTime       = 0x4C;
        constexpr uintptr_t m_bEligibleForScreenHighlight = 0x50;
        constexpr uintptr_t m_bGlowing              = 0x51;
    }
}
