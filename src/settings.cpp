#include "settings.h"
#include "MainKratosCombat.h"
//#define DefaultConfigPath "Data\\MCM\\Config\\KratosCombat\\KratosCombat.ini"
//#define UserConfigPath "Data\\MCM\\Settings\\KratosCombat\\KratosCombat.ini"

void Config::ReadStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string& a_setting)
{
    const char* bFound = nullptr;
    bFound = a_ini.GetValue(a_sectionName, a_settingName);
    if (bFound) {
        a_setting = a_ini.GetValue(a_sectionName, a_settingName);
    }
}
void Config::ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting)
{
    const char* bFound = nullptr;
    bFound = a_ini.GetValue(a_sectionName, a_settingName);
    if (bFound) {
        a_setting = static_cast<float>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
    }
}
void Config::ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting)
{
    const char* bFound = nullptr;
    bFound = a_ini.GetValue(a_sectionName, a_settingName);
    if (bFound) {
        a_setting = static_cast<int>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
    }
}
void Config::WriteIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t a_setting)
{
    a_ini.SetDoubleValue(a_sectionName, a_settingName, a_setting, nullptr, true);
//    const char* bFound = nullptr;
//    bFound = a_ini.GetValue(a_sectionName, a_settingName);
//    if (bFound) {
//        a_ini.SetDoubleValue(a_sectionName, a_settingName, a_setting, nullptr, true);
//    }
}

void Config::ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting)
{
    const char* bFound = nullptr;
    bFound = a_ini.GetValue(a_sectionName, a_settingName);
    if (bFound) {
        a_setting = a_ini.GetBoolValue(a_sectionName, a_settingName);
    }
}
/**/
void Config::ReadConfig(std::filesystem::path a_path, const bool a_writeChanges)
{
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(a_path.string().c_str());

    if (a_writeChanges) {
//        Config::WriteIntSetting(ini, "SpecialEquipments", "uSpecialWeapon", (uint32_t&)SpecialWeapon);
//        Config::WriteIntSetting(ini, "SpecialEquipments", "uSpecialShield", (uint32_t&)SpecialShield);
        ini.SaveFile(a_path.string().c_str());
        ini.LoadFile(a_path.string().c_str());
        return;
    }

//  ReadIntSetting(ini, "Main", "iAimButtonID", InputEventTracker::AimButton);
//  ReadIntSetting(ini, "Main", "iCallingButtonID", InputEventTracker::AxeCallButton);

    ReadFloatSetting(ini, "Main", "fBarehandedDamageMult", BarehandedDamageMult);

    ReadFloatSetting(ini, "Main", "fThrowSpeed", ThrowSpeed);
    ReadFloatSetting(ini, "Main", "fThrowRotationSpeed", ThrowRotationSpeed);

    ReadFloatSetting(ini, "Main", "fArrivalTime", ArrivalTime);
    ReadFloatSetting(ini, "Main", "fArrivalRotationSpeed", ArrivalRotationSpeed);
    ReadFloatSetting(ini, "Main", "fArrivalRotationX", ArrivalRotationX);
    ReadFloatSetting(ini, "Main", "fArrivalRotationY", ArrivalRotationY);
    ReadFloatSetting(ini, "Main", "fArrivalRotationZ", ArrivalRotationZ);
    ReadFloatSetting(ini, "Main", "fArrivalRoadCurveMagnitude", ArrivalRoadCurveMagnitude);
    ReadFloatSetting(ini, "Main", "fCatchingTreshold", CatchingTreshold);

    ReadFloatSetting(ini, "Main", "fMaxArrivalSpeed", MaxArrivalSpeed);
    ReadFloatSetting(ini, "Main", "fMinArrivalSpeed", MinArrivalSpeed);
    ReadFloatSetting(ini, "Main", "fMaxAxeStuckAngle", MaxAxeStuckAngle);
    ReadFloatSetting(ini, "Main", "fMinAxeStuckAngle", MinAxeStuckAngle);

    ReadIntSetting(ini, "Main", "iDraupnirExplodableCount", DraupnirSpearCount);
    ReadFloatSetting(ini, "Main", "fDraupnirExplosionsInterval", DraupnirExplosionsInterval);
    
    ReadIntSetting(ini, "Main", "iChargeHitCount", ChargeHitCount);
    ReadFloatSetting(ini, "Main", "fChargeMagnitude", ChargeMagnitude);

    ReadStringSetting(ini, "Main", "sKratosCombatESP", KratosCombatESP);
    ReadStringSetting(ini, "Main", "sDraupnirModESP", DraupnirModESP);

    ReadStringSetting(ini, "Main", "sWeaponThrowingEvent", ThrowEvent);
    ReadStringSetting(ini, "Main", "sWeaponCallingEvent", CallEvent);
    ReadStringSetting(ini, "Main", "sAxeChargeEvent", ChargeLeviEvent);
    ReadStringSetting(ini, "Main", "sDraupnirsCallEvent", DraupnirsCallEvent);

    ReadIntSetting(ini, "Rage", "uRageType", (uint32_t&)RageType);
    ReadBoolSetting(ini, "Rage", "bRageBarOpen", RageBarOpen);
    ReadIntSetting(ini, "Rage", "uRageBarRot", RageBarRot);
    ReadIntSetting(ini, "Rage", "uRageBarPosX", RageBarPosX);
    ReadIntSetting(ini, "Rage", "uRageBarPosY", RageBarPosY);
    ReadIntSetting(ini, "Rage", "uRageBarScaleX", RageBarScaleX);
    ReadIntSetting(ini, "Rage", "uRageBarScaleY", RageBarScaleY);
    ReadIntSetting(ini, "Rage", "uRageBarTransparency", RageBarTransparency);
    
    if (gRageBarOpen && gRageBarRot && gRageBarPosX && gRageBarPosY && gRageBarScaleX && gRageBarScaleY && gRageBarTransparency) {
        gRageBarOpen->value         = (float)RageBarOpen;
        gRageBarRot->value          = (float)RageBarRot;
        gRageBarPosX->value         = (float)RageBarPosX;
        gRageBarPosY->value         = (float)RageBarPosY;
        gRageBarScaleX->value       = (float)RageBarScaleX;
        gRageBarScaleY->value       = (float)RageBarScaleY;
        gRageBarTransparency->value = (float)RageBarTransparency;
    }

    ReadIntSetting(ini, "Keybindings", "iAxeCallKey", AxeCallKey);
    ReadIntSetting(ini, "Keybindings", "iAimKey", AimKey);
    ReadIntSetting(ini, "Keybindings", "iRunicKey", RunicKey);
    ReadIntSetting(ini, "Keybindings", "iFinisherKey", FinisherKey);
    ReadIntSetting(ini, "Keybindings", "iMediumDistanceKey", MediumDistanceKey);
    ReadIntSetting(ini, "Keybindings", "iLongDistanceKey", LongDistanceKey);

    ReadBoolSetting(ini, "Main", "bDontHitWhileArriving", DontDamageWhileArrive);
    ReadBoolSetting(ini, "Misc", "bDebugModeOpen", DebugModeOpen);

    if      (ThrowSpeed < 1000.f)   ThrowSpeed = 1000.f;
    else if (ThrowSpeed > 100000.f) ThrowSpeed = 100000.f;

    if      (ArrivalTime <= 0.f)    ArrivalTime = 0.05f;
    else if (ArrivalTime > 60.f)    ArrivalTime = 60.f;

    if      (DraupnirSpearCount > 9)    DraupnirSpearCount = 9;
    else if (DraupnirSpearCount < 1)    DraupnirSpearCount = 1;
    DraupnirSpearCount -= 1;                // DraupnirSpearCount starts with 0

    for (int i = Config::DraupnirSpearCount; i <= 9; i++) {
        if (Draupnir::DraupnirSpearProjectiles[i] != nullptr) {
            Draupnir::DraupnirSpearProjectiles[i] = nullptr;}
        if (Draupnir::DraupnirSpearHitBones[i] != nullptr) {
            Draupnir::DraupnirSpearHitBones[i] = nullptr;}
        //  spdlog::debug("Draupnir spear {} is nullptr", i);
    }

    if (ArrivalRoadCurveMagnitude > 100.f)  ArrivalRoadCurveMagnitude = 100.f;
    if (ArrivalRoadCurveMagnitude < -100.f) ArrivalRoadCurveMagnitude = -100.f;

    if (DebugModeOpen) {
        spdlog::set_level(spdlog::level::trace);
        spdlog::debug("Debug mode enabled");
    } else spdlog::set_level(spdlog::level::info);
}
inline void PrepareValues()
{
    Config::ThrowRotationSpeed *= 0.017453292f;

    Config::ArrivalRoadCurveMagnitude *= 0.017453292f;
    Config::ArrivalRoadCurveMagnitude = sinf(Config::ArrivalRoadCurveMagnitude);

    Config::ArrivalRotationSpeed *= 0.017453292f;
    Config::ArrivalRotationX *= 0.017453292f;
    Config::ArrivalRotationY *= 0.017453292f;
    Config::ArrivalRotationZ *= 0.017453292f;

    Config::MinAxeStuckAngle *= 0.017453292f;
    Config::MaxAxeStuckAngle *= 0.017453292f;
}
void Config::CheckConfig(const bool a_writeChanges)
{
    constexpr auto DefaultConfigPath = L"Data/MCM/Config/KratosCombat/Settings.ini";
    constexpr auto UserConfigPath = L"Data/MCM/Settings/KratosCombat.ini";
    if (!a_writeChanges) ReadConfig(DefaultConfigPath);
    ReadConfig(UserConfigPath, a_writeChanges);
    PrepareValues();
        spdlog::info("Kratos Combat's configurations checked.");
}

bool Config::CheckForms()
{
    bool found = true;
    auto dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {spdlog::error("can't get data handler!!"); return false;}

    //  Globals
    SpecialWeapon = dataHandler->LookupForm<RE::TESGlobal>(0x82B, Config::KratosCombatESP);
    SpecialShield = dataHandler->LookupForm<RE::TESGlobal>(0x82C, Config::KratosCombatESP);
    if (!SpecialWeapon || !SpecialShield) {spdlog::error("Can't find Special Equipment globals"); found = false;}
    
    RageLimit       = dataHandler->LookupForm<RE::TESGlobal>(0x831, Config::KratosCombatESP);
    RageAmount      = dataHandler->LookupForm<RE::TESGlobal>(0x82E, Config::KratosCombatESP);
    RageBuffRate    = dataHandler->LookupForm<RE::TESGlobal>(0x82F, Config::KratosCombatESP);
    RageDamageRate  = dataHandler->LookupForm<RE::TESGlobal>(0x830, Config::KratosCombatESP);
    if (!RageLimit || !RageAmount || !RageBuffRate || !RageDamageRate) {spdlog::error("Can't find Rage related globals"); found = false;}

    gRageBarOpen         = dataHandler->LookupForm<RE::TESGlobal>(0x838, Config::KratosCombatESP);
    gRageBarRot          = dataHandler->LookupForm<RE::TESGlobal>(0x836, Config::KratosCombatESP);
    gRageBarPosX         = dataHandler->LookupForm<RE::TESGlobal>(0x834, Config::KratosCombatESP);
    gRageBarPosY         = dataHandler->LookupForm<RE::TESGlobal>(0x835, Config::KratosCombatESP);
    gRageBarScaleX       = dataHandler->LookupForm<RE::TESGlobal>(0x832, Config::KratosCombatESP);
    gRageBarScaleY       = dataHandler->LookupForm<RE::TESGlobal>(0x833, Config::KratosCombatESP);
    gRageBarTransparency = dataHandler->LookupForm<RE::TESGlobal>(0x837, Config::KratosCombatESP);
    if (!gRageBarOpen || !gRageBarRot || !gRageBarPosX || !gRageBarPosY || !gRageBarScaleX || !gRageBarScaleY || !gRageBarTransparency)
        {spdlog::error("Can't find RageBar related globals"); found = false;}

    //  The Leviathan Axe's stuff
    auto Levi = LeviathanAxe::GetSingleton();
    found = Levi->Initialize();
    //  The Draupnir Spear's stuff
    auto draupnir = Draupnir::GetSingleton();
    found = draupnir->Initialize();

    auto kratos = Kratos::GetSingleton();
    found = kratos->Initialize();

    LeviathanAxeKWD     = dataHandler->LookupForm<RE::BGSKeyword>(0x817, Config::KratosCombatESP);
    BladeOfChaosKWD     = dataHandler->LookupForm<RE::BGSKeyword>(0x818, Config::KratosCombatESP);
    DraupnirSpearKWD    = dataHandler->LookupForm<RE::BGSKeyword>(0x819, Config::KratosCombatESP);
    BladeOfOlympusKWD   = dataHandler->LookupForm<RE::BGSKeyword>(0x81A, Config::KratosCombatESP);
    GuardianShieldKWD   = dataHandler->LookupForm<RE::BGSKeyword>(0x82D, Config::KratosCombatESP);
    if (LeviathanAxeKWD && BladeOfChaosKWD && DraupnirSpearKWD && BladeOfOlympusKWD && GuardianShieldKWD)
        SpecialKWDs = {Config::LeviathanAxeKWD, Config::BladeOfChaosKWD, Config::DraupnirSpearKWD, Config::BladeOfOlympusKWD, Config::GuardianShieldKWD};
    else {spdlog::error("Check Kratos's keyword addresses"); found = false;}

    return found;
}
/**/
void APIs::Request()
{
        spdlog::info("API's loading...");
//  if (!tdm) {
//      tdm = reinterpret_cast<TDM_API::IVTDM1*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V1));
//      if (tdm) spdlog::debug("TDM API loaded");
//      else spdlog::error("TDM API failed to load");
//  }
}