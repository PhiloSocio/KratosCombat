#include "settings.h"
#include "MainKratosCombat.h"
//#define DefaultConfigPath "Data\\MCM\\Config\\KratosCombat\\KratosCombat.ini"
//#define UserConfigPath "Data\\MCM\\Settings\\KratosCombat\\KratosCombat.ini"

void Config::ReadStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string& a_setting)
{
    const char* bFound = nullptr;
    bFound = a_ini.GetValue(a_sectionName, a_settingName);
    if (bFound) {
        a_setting = bFound;
    }// else spdlog::warn("string setting {} can't found for reading!", a_settingName);
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
        if (std::strncmp(bFound, "0x", 2) == 0)  a_setting = static_cast<uint32_t>(std::strtoul(bFound, nullptr, 16));
        else a_setting = static_cast<uint32_t>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
    }
}
void Config::ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting)
{
    const char* bFound = nullptr;
    bFound = a_ini.GetValue(a_sectionName, a_settingName);
    if (bFound) {
        a_setting = a_ini.GetBoolValue(a_sectionName, a_settingName);
    }
}
void Config::WriteStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string a_setting)
{
    a_ini.SetValue(a_sectionName, a_settingName, a_setting.c_str(), nullptr, true);
//    const char* bFound = nullptr;
//    bFound = a_ini.GetValue(a_sectionName, a_settingName);
//    if (bFound) {
//        spdlog::debug("string setting {} is writed as {}", a_settingName, bFound);
//    } else spdlog::warn("string setting {} can't be found for writing!", a_settingName);
}
void Config::WriteIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t a_setting, const bool a_isHex)
{
    if (a_isHex) {
        char hexString[16];
        snprintf(hexString, sizeof(hexString), "0x%X", a_setting);
        a_ini.SetValue(a_sectionName, a_settingName, hexString, nullptr, true);
    } else a_ini.SetDoubleValue(a_sectionName, a_settingName, a_setting, nullptr, true);
//    const char* bFound = nullptr;
//    bFound = a_ini.GetValue(a_sectionName, a_settingName);
//    if (bFound) {
//        spdlog::debug("int setting {} is writed as {}", a_settingName, bFound);
//    } else spdlog::warn("int setting {} can't be found for writing!", a_settingName);
}
/**/
void Config::ReadConfig(std::filesystem::path a_path, const bool a_writeChanges)
{
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(a_path.string().c_str());

    if (a_writeChanges) {
        WriteIntSetting(ini, "Registration", "uRegisteredLeviathanID",      (uint32_t)registeredLeviathanID, true);
        WriteIntSetting(ini, "Registration", "uRegisteredBladeOfChaosID",   (uint32_t)registeredBladeOfChaosID, true);
        WriteIntSetting(ini, "Registration", "uRegisteredLBladeOfChaosID",  (uint32_t)registeredLBladeOfChaosID, true);
        WriteIntSetting(ini, "Registration", "uRegisteredDraupnirID",       (uint32_t)registeredDraupnirID, true);
        WriteIntSetting(ini, "Registration", "uRegisteredBladeOfOlympusID", (uint32_t)registeredBladeOfOlympusID, true);
        WriteIntSetting(ini, "Registration", "uRegisteredMjolnirID",        (uint32_t)registeredMjolnirID, true);
        WriteIntSetting(ini, "Registration", "uRegisteredTridentID",        (uint32_t)registeredTridentID, true);
        WriteIntSetting(ini, "Registration", "uRegisteredGuardianShieldID", (uint32_t)registeredGuardianShieldID, true);
        WriteStringSetting(ini, "Registration", "sRegisteredLeviathanFileName",         registeredLeviathanFileName);
        WriteStringSetting(ini, "Registration", "sRegisteredBladeOfChaosFileName",      registeredBladeOfChaosFileName);
        WriteStringSetting(ini, "Registration", "sRegisteredLBladeOfChaosFileName",     registeredLBladeOfChaosFileName);
        WriteStringSetting(ini, "Registration", "sRegisteredDraupnirFileName",          registeredDraupnirFileName);
        WriteStringSetting(ini, "Registration", "sRegisteredBladeOfOlympusFileName",    registeredBladeOfOlympusFileName);
        WriteStringSetting(ini, "Registration", "sRegisteredMjolnirFileName",           registeredMjolnirFileName);
        WriteStringSetting(ini, "Registration", "sRegisteredTridentFileName",           registeredTridentFileName);
        WriteStringSetting(ini, "Registration", "sRegisteredGuardianShieldFileName",    registeredGuardianShieldFileName);

        ini.SaveFile(a_path.string().c_str());
        ini.LoadFile(a_path.string().c_str());
    }

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

//    ReadStringSetting(ini, "Main", "sKratosCombatESP", KratosCombatESP);
//    ReadStringSetting(ini, "Main", "sDraupnirModESP", DraupnirModESP);

//    ReadStringSetting(ini, "Main", "sWeaponThrowingEvent", ThrowEvent);
//    ReadStringSetting(ini, "Main", "sWeaponCallingEvent", CallEvent);
//    ReadStringSetting(ini, "Main", "sAxeChargeEvent", ChargeLeviEvent);
//    ReadStringSetting(ini, "Main", "sDraupnirsCallEvent", DraupnirsCallEvent);

    ReadIntSetting(ini, "Registration", "uRegisteredLeviathanID",      (uint32_t&)registeredLeviathanID);
    ReadIntSetting(ini, "Registration", "uRegisteredBladeOfChaosID",   (uint32_t&)registeredBladeOfChaosID);
    ReadIntSetting(ini, "Registration", "uRegisteredLBladeOfChaosID",  (uint32_t&)registeredLBladeOfChaosID);
    ReadIntSetting(ini, "Registration", "uRegisteredDraupnirID",       (uint32_t&)registeredDraupnirID);
    ReadIntSetting(ini, "Registration", "uRegisteredBladeOfOlympusID", (uint32_t&)registeredBladeOfOlympusID);
    ReadIntSetting(ini, "Registration", "uRegisteredMjolnirID",        (uint32_t&)registeredMjolnirID);
    ReadIntSetting(ini, "Registration", "uRegisteredTridentID",        (uint32_t&)registeredTridentID);
    ReadIntSetting(ini, "Registration", "uRegisteredGuardianShieldID", (uint32_t&)registeredGuardianShieldID);
    ReadStringSetting(ini, "Registration", "sRegisteredLeviathanFileName",         registeredLeviathanFileName);
    ReadStringSetting(ini, "Registration", "sRegisteredBladeOfChaosFileName",      registeredBladeOfChaosFileName);
    ReadStringSetting(ini, "Registration", "sRegisteredLBladeOfChaosFileName",     registeredLBladeOfChaosFileName);
    ReadStringSetting(ini, "Registration", "sRegisteredDraupnirFileName",          registeredDraupnirFileName);
    ReadStringSetting(ini, "Registration", "sRegisteredBladeOfOlympusFileName",    registeredBladeOfOlympusFileName);
    ReadStringSetting(ini, "Registration", "sRegisteredMjolnirFileName",           registeredMjolnirFileName);
    ReadStringSetting(ini, "Registration", "sRegisteredTridentFileName",           registeredTridentFileName);
    ReadStringSetting(ini, "Registration", "sRegisteredGuardianShieldFileName",    registeredGuardianShieldFileName);

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

    if (DebugModeOpen && spdlog::get_level() >= spdlog::level::info) {
        spdlog::set_level(spdlog::level::trace);
        spdlog::debug("Debug mode enabled");
    } else {
        spdlog::set_level(spdlog::level::info);
        spdlog::info("Debug mode closed");
    }
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
    ReadConfig(DefaultConfigPath);
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
    //  The Mjolnir's stuff
    auto mjolnir = Mjolnir::GetSingleton();
    found = mjolnir->Initialize();

#ifdef TRIDENT
    //  The Trident's stuff
    auto trident = Trident::GetSingleton();
    found = trident->Initialize();
#endif

    auto kratos = Kratos::GetSingleton();
    found = kratos->Initialize();

    LeviathanAxeKWD     = dataHandler->LookupForm<RE::BGSKeyword>(0x817, Config::KratosCombatESP);
    BladeOfChaosKWD     = dataHandler->LookupForm<RE::BGSKeyword>(0x818, Config::KratosCombatESP);
    DraupnirSpearKWD    = dataHandler->LookupForm<RE::BGSKeyword>(0x819, Config::KratosCombatESP);
    BladeOfOlympusKWD   = dataHandler->LookupForm<RE::BGSKeyword>(0x81A, Config::KratosCombatESP);
    MjolnirKWD          = dataHandler->LookupForm<RE::BGSKeyword>(0x83F, Config::KratosCombatESP);
    TridentKWD          = dataHandler->LookupForm<RE::BGSKeyword>(0x808, Config::TridentModESP);
    GuardianShieldKWD   = dataHandler->LookupForm<RE::BGSKeyword>(0x82D, Config::KratosCombatESP);
    if (LeviathanAxeKWD && BladeOfChaosKWD && DraupnirSpearKWD && BladeOfOlympusKWD && GuardianShieldKWD)
        SpecialKWDs = {Config::LeviathanAxeKWD, Config::BladeOfChaosKWD, Config::DraupnirSpearKWD, Config::BladeOfOlympusKWD, Config::MjolnirKWD, Config::TridentKWD, Config::GuardianShieldKWD};
    else {spdlog::error("Check Kratos's keyword addresses"); found = false;}

    return found;
}
/**/
std::optional<bool> APIs::Request()
{
    std::optional<bool> result;
//  if (!tdm) {
//      tdm = reinterpret_cast<TDM_API::IVTDM1*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V1));
//      if (tdm) spdlog::debug("TDM API loaded");
//      else spdlog::error("TDM API failed to load");
//  }
    if (!precision) {
        precision = reinterpret_cast<PRECISION_API::IVPrecision4*>(PRECISION_API::RequestPluginAPI(PRECISION_API::InterfaceVersion::V4));
        if (precision) {
            result = true;
            spdlog::info("Requesting Precision API ok");
        } else {
            spdlog::warn("Requesting Precision API not ok");
        }
    } else {
        result = true;
        spdlog::debug("Precision API already loaded");
    }
    return result;
}