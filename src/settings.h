#pragma once
#include "SimpleIni.h"

class Config
{
public:
    //Kratos
    static inline float BarehandedDamageMult        = 10.f;
    static inline bool BuffRageWhenDamaged          = false;
    //Axe throw
    static inline float ThrowSpeed                  = 3600.f;       //  cm/s
    static inline float ThrowRotationSpeed          = 2160.f;       //  deg/s
    static inline float NoGravityDurationLeviathan  = 2.f;
    static inline std::string ThrowEvent            = "ThrowWeapon";
    static inline std::string ThrowEventV           = "ThrowWeaponV";
    static inline std::string ThrowEndEvent         = "ThrowAttackEnd";

    //Axe call
    static inline float ArrivalTime                 = 0.52f;        //  s
    static inline float ArrivalRotationSpeed        = 720.f;        //  deg/s
    static inline float ArrivalRotationX            = 90.f;         //  deg/s
    static inline float ArrivalRotationY            = 0.f;          //  deg/s
    static inline float ArrivalRotationZ            = 180.f;            //  deg/s
    static inline float ArrivalRoadCurveMagnitude   = 18.f;         //  deg
    static inline float CatchingTreshold            = 69.f;         //  cm
    static inline std::string CallEvent             = "CallWeapon";
    static inline std::string CallAttackEvent       = "LeviCallAttack";
    static inline std::string ChargeLeviEvent       = "LeviCharge";
    static inline bool  DontDamageWhileArrive       = false;

    //Limit values
    static inline float MaxArrivalSpeed             = 60000.f;      //  cm/s
    static inline float MinArrivalSpeed             = 1600.f;       //  cm/s

    static inline float MaxAxeStuckAngle            = 350.f;        //  cm/s
    static inline float MinAxeStuckAngle            = 80.f;         //  cm/s

    //Charging system
    static inline uint32_t ChargeHitCount           = 5;            //  charge will be reset when this count of hits is made
    static inline float ChargeMagnitude             = 1.8f;         //  multiplies the magnitude of enchantment if your weapon is already enchanted

    //Draupnir spear
    static inline float DraupnirExplosionsInterval  = 0.24f;
    static inline uint32_t DraupnirSpearCount       = 9;            //  count of spear explodable projectiles (max 9)
    static inline float NoGravityDurationDraupnir   = 1.f;
    static inline std::string DraupnirsCallEvent    = "DraupnirsCall";
    static inline std::string SpearThrowEvent       = "ThrowSpear";

    //Weapon register values
    static inline RE::BGSKeyword* LeviathanAxeKWD   = nullptr;
    static inline RE::BGSKeyword* BladeOfChaosKWD   = nullptr;
    static inline RE::BGSKeyword* DraupnirSpearKWD  = nullptr;
    static inline RE::BGSKeyword* BladeOfOlympusKWD = nullptr;
    static inline RE::BGSKeyword* GuardianShieldKWD = nullptr;
    static inline std::vector<RE::BGSKeyword *> SpecialKWDs = {};

    static inline RE::TESGlobal* SpecialWeapon      = nullptr;
    static inline RE::TESGlobal* LastChargedLevi    = nullptr;
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
    static inline bool RageBarOpen              = 1;
    static inline uint32_t RageBarRot           = 0;
    static inline uint32_t RageBarPosX          = 100;
    static inline uint32_t RageBarPosY          = 500;
    static inline uint32_t RageBarScaleX        = 50;
    static inline uint32_t RageBarScaleY        = 50;
    static inline uint32_t RageBarTransparency  = 0;
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

    static inline bool  DebugModeOpen       = true;

    static void CheckConfig(const bool a_writeChanges = false);
    static bool CheckForms();
private:
    static void ReadConfig(std::filesystem::path a_path, const bool a_writeChanges = false);
    static void ReadStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string& a_setting);
    static void ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting);
    static void ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting);
    static void ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting);
    static void WriteIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t a_setting);
};
/**/
class APIs
{
public:
//  static inline TDM_API::IVTDM1* tdm = nullptr;
//  static inline TDM_API::APIResult result;

    static void Request();
};