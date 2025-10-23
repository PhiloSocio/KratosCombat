#pragma once
#include "SimpleIni.h"
#include "API/PrecisionAPI.h"

class Config
{
public:
    //Kratos
    static inline float BarehandedDamageMult        = 10.f;
    static inline bool BuffRageWhenDamaged          = false;
    //Axe-Mace throw
    static inline float ThrowSpeed                  = 3700.f;       //  cm/s
    static inline float ThrowRotationSpeed          = 2160.f;       //  deg/s
    static inline float NoGravityDurationLeviathan  = 0.36f;
    static inline float NoGravityDurationMjolnir    = 0.36f;
    static inline std::string ThrowEvent            = "ThrowWeapon";
    static inline std::string ThrowEventV           = "ThrowWeaponV";
    static inline std::string ThrowEndEvent         = "ThrowAttackEnd";
    //Homing mechanic: The homing projectile will turn around the player and hit all combat targets
    static inline float HProjectileTargetRange      = 1800.f;       //  cm
    static inline float HProjectileSpeed            = 2000.f;       //  cm/s
    static inline uint32_t HProjectileLoopCount     = 2u;           //  how many times it will hit each combat target.
    static inline bool ReturnHProjectileAfterLoops  = true;
    //Trails
    static inline bool DrawTrails                   = true;         //  draw trails for thrown weapons
    static inline uint8_t TrailParticleCount        = 6u;           //  how many particles will be used for the trail
    static inline std::string TrailModelPathLevi    = "effects/WeaponTrails/AttackTrailMagic_Frost.nif";
    static inline std::string TrailModelPathMjolnir = "effects/WeaponTrails/AttackTrailMagic_Shock.nif";
    
    //Requirements
    static inline bool IsSkipEquipAnimationInstalled = false;
    static inline bool IsAdvancedThrowingInstalled = false;
    static inline bool IsPrecisionInstalled = false;
    static inline bool IsTrueDirectionalMovementInstalled = false;

    //Axe/Mace call
    static inline float ArrivalTime                 = 0.52f;        //  s
    static inline float ArrivalRotationSpeed        = 1440.f;       //  deg/s
    static inline float ArrivalRotationX            = 90.f;         //  deg/s
    static inline float ArrivalRotationY            = 0.f;          //  deg/s
    static inline float ArrivalRotationZ            = 180.f;        //  deg/s
    static inline float ArrivalRoadCurveMagnitude   = 18.f;         //  deg
    static inline float CatchingTreshold            = 69.f;         //  cm
    static inline float ArrivalAngleSnap            = 0.1f;         //  snap to nearest angle in radians
    static inline std::optional<float> MjolnirArrivingDelay = std::nullopt;
    static inline std::string CallEvent             = "CallWeapon";
    static inline std::string CallAttackEvent       = "LeviCallAttack";
    static inline std::string ChargeLeviEvent       = "LeviCharge";
    static inline bool  DontDamageWhileArrive       = false;

    //Limit values
    static inline float MaxArrivalSpeed             = 69000.f;      //  cm/s
    static inline float MinArrivalSpeed             = 1669.f;       //  cm/s

    static inline float MaxAxeStuckAngle            = 350.f;        //  cm/s
    static inline float MinAxeStuckAngle            = 80.f;         //  cm/s

    //Charging system
    static inline uint32_t ChargeHitCount           = 1;            //  charge will be reset when this count of hits is made
    static inline float ChargeMagnitude             = 1.8f;         //  multiplies the magnitude of enchantment if your weapon is already enchanted

    //Draupnir spear
    static inline float DraupnirExplosionsInterval  = 0.24f;
    static inline uint32_t DraupnirSpearCount       = 6;            //  count of spear explodable projectiles (max 9)
    static inline float NoGravityDurationDraupnir   = 0.36f;
    static inline std::string DraupnirsCallEvent    = "DraupnirsCall";
    static inline std::string SpearThrowEvent       = "ThrowSpear";

    //Weapon register values
    static inline RE::BGSKeyword* LeviathanAxeKWD   = nullptr;
    static inline RE::BGSKeyword* BladeOfChaosKWD   = nullptr;
    static inline RE::BGSKeyword* DraupnirSpearKWD  = nullptr;
    static inline RE::BGSKeyword* BladeOfOlympusKWD = nullptr;
    static inline RE::BGSKeyword* MjolnirKWD        = nullptr;
    static inline RE::BGSKeyword* TridentKWD        = nullptr;
    static inline RE::BGSKeyword* GuardianShieldKWD = nullptr;
    static inline std::vector<RE::BGSKeyword *> SpecialKWDs = {};

    static inline RE::TESGlobal* SpecialWeapon      = nullptr;
    static inline RE::TESGlobal* SpecialShield      = nullptr;

    static inline RE::TESGlobal* RageLimit          = nullptr;
    static inline RE::TESGlobal* RageAmount         = nullptr;
    static inline RE::TESGlobal* RageBuffRate       = nullptr;
    static inline RE::TESGlobal* RageDamageRate     = nullptr;

    static inline RE::TESGlobal* gRageBarOpen        = nullptr;
    static inline RE::TESGlobal* gRageBarRot         = nullptr;
    static inline RE::TESGlobal* gRageBarPosX        = nullptr;
    static inline RE::TESGlobal* gRageBarPosY        = nullptr;
    static inline RE::TESGlobal* gRageBarScaleX      = nullptr;
    static inline RE::TESGlobal* gRageBarScaleY      = nullptr;
    static inline RE::TESGlobal* gRageBarTransparency= nullptr;
    static inline bool RageBarOpen              = true;
    static inline uint32_t RageBarRot           = 0;
    static inline uint32_t RageBarPosX          = 137;
    static inline uint32_t RageBarPosY          = 645;
    static inline uint32_t RageBarScaleX        = 50;
    static inline uint32_t RageBarScaleY        = 50;
    static inline uint32_t RageBarTransparency  = 69;

    static inline RE::FormID registeredLeviathanID      = 105;
    static inline RE::FormID registeredBladeOfChaosID   = 105;
    static inline RE::FormID registeredLBladeOfChaosID  = 105;
    static inline RE::FormID registeredDraupnirID       = 105;
    static inline RE::FormID registeredBladeOfOlympusID = 105;
    static inline RE::FormID registeredMjolnirID        = 105;
    static inline RE::FormID registeredTridentID        = 105;
    static inline RE::FormID registeredGuardianShieldID = 105;
    static inline std::string registeredLeviathanFileName      = "Not Registered";
    static inline std::string registeredBladeOfChaosFileName   = "Not Registered";
    static inline std::string registeredLBladeOfChaosFileName  = "Not Registered";
    static inline std::string registeredDraupnirFileName       = "Not Registered";
    static inline std::string registeredBladeOfOlympusFileName = "Not Registered";
    static inline std::string registeredMjolnirFileName        = "Not Registered";
    static inline std::string registeredTridentFileName        = "Not Registered";
    static inline std::string registeredGuardianShieldFileName = "Not Registered";
    //Rage
    static inline uint8_t RageType = 0;

    //Keybindings
    static inline uint32_t AxeCallKey         = -1;
    static inline uint32_t AimKey             = -1;
    static inline uint32_t RunicKey           = -1;
    static inline uint32_t FinisherKey        = -1;
    static inline uint32_t MediumDistanceKey  = -1;
    static inline uint32_t LongDistanceKey    = -1;
    static inline uint32_t PowerAttackKey     = -1;

    static inline std::string DraupnirModESP    = "Draupnir Spear Mechanic.esp";
    static inline std::string KratosCombatESP   = "KratosCombat.esp";
    static inline std::string TridentModESP     = "KratosCombat-Trident.esp";

    static inline bool  DebugModeOpen       = true;

    static void CheckConfig(const bool a_writeChanges = false);
    static bool CheckForms();
private:
    static void ReadConfig(std::filesystem::path a_path, const bool a_writeChanges = false);
    static void ReadStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string& a_setting);
    static void ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting);
    static void ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting);
    static void ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting);
    static void WriteStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string a_setting);
    static void WriteIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t a_setting, const bool a_isHex = false);
};
/**/
class APIs
{
public:
//  static inline TDM_API::IVTDM1* tdm = nullptr;
//  static inline TDM_API::APIResult tdmResult;
    static inline PRECISION_API::IVPrecision4* precision = nullptr;
    static inline PRECISION_API::APIResult precisionResult;

    static std::optional<bool> Request();
};