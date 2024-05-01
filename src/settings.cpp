#include "settings.h"
#include "MainKratosCombat.h"
#define ConfigPath "Data\\SKSE\\Plugins\\KratosCombat.ini"

void Config::ReadStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string& a_setting)
{
    const char* bFound = nullptr;
    bFound = a_ini.GetValue(a_sectionName, a_settingName);
    if (bFound) {
        //spdlog::info("found {} with value {}", a_settingName, bFound);
        a_setting = a_ini.GetValue(a_sectionName, a_settingName);
    }
}

void Config::ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting)
{
    const char* bFound = nullptr;
    bFound = a_ini.GetValue(a_sectionName, a_settingName);
    if (bFound) {
        //spdlog::info("found {} with value {}", a_settingName, bFound);
        a_setting = static_cast<float>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
    }
}

void Config::ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting)
{
    const char* bFound = nullptr;
    bFound = a_ini.GetValue(a_sectionName, a_settingName);
    if (bFound) {
        //spdlog::info("found {} with value {}", a_settingName, bFound);
        a_setting = static_cast<int>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
    }
}
/*
void Config::ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting)
{
    const char* bFound = nullptr;
    bFound = a_ini.GetValue(a_sectionName, a_settingName);
    if (bFound) {
        //spdlog::info("found {} with value {}", a_settingName, bFound);
        a_setting = a_ini.GetBoolValue(a_sectionName, a_settingName);
    }
}
*/

void Config::CheckConfig()
{
    CSimpleIniA ini;
    ini.LoadFile(ConfigPath);

//  ReadIntSetting(ini, "Main", "iAimButtonID", InputEventTracker::AimButton);
//  ReadIntSetting(ini, "Main", "iCallingButtonID", InputEventTracker::AxeCallButton);

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
    ReadFloatSetting(ini, "Main", "fProjectileCastingDelay", ProjCastDelay);
    
    ReadIntSetting(ini, "Main", "iChargeHitCount", ChargeHitCount);
    ReadFloatSetting(ini, "Main", "fChargeMagnitude", ChargeMagnitude);

    ReadStringSetting(ini, "Main", "sLeviathanModESP", LeviathanModESP);
    ReadStringSetting(ini, "Main", "sDraupnirModESP", DraupnirModESP);

    ReadStringSetting(ini, "Main", "sWeaponThrowingEvent", ThrowEvent);
    ReadStringSetting(ini, "Main", "sWeaponCallingEvent", CallEvent);
    ReadStringSetting(ini, "Main", "sAxeChargeEvent", ChargeLeviEvent);
    ReadStringSetting(ini, "Main", "sDraupnirsCallEvent", DraupnirsCallEvent);

    DontDamageWhileArrive = ini.GetBoolValue("Main", "bDontHitWhileArriving");

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

    ThrowRotationSpeed *= 0.017453292f;

    ArrivalRoadCurveMagnitude *= 0.017453292f;
    ArrivalRoadCurveMagnitude = sinf(ArrivalRoadCurveMagnitude);

    ArrivalRotationSpeed *= 0.017453292f;
    ArrivalRotationX *= 0.017453292f;
    ArrivalRotationY *= 0.017453292f;
    ArrivalRotationZ *= 0.017453292f;

    MinAxeStuckAngle *= 0.017453292f;
    MaxAxeStuckAngle *= 0.017453292f;

    if (ini.GetBoolValue("Main", "DebugModeOpen")) {
        spdlog::set_level(spdlog::level::trace);
        spdlog::debug("Debug mode enabled");
    } else spdlog::set_level(spdlog::level::info);
    spdlog::info("Kratos Combat's configurations checked.");
}

void Config::CheckProjectiles()
{
    auto dataHandler = RE::TESDataHandler::GetSingleton();

    //  The Leviathan Axe's stuff
    auto Levi = LeviathanAxe::GetSingleton();
    Levi->LeviProjBaseL     = dataHandler->LookupForm<RE::BGSProjectile>    (0x815, Config::LeviathanModESP);
    Levi->LeviProjBaseH     = dataHandler->LookupForm<RE::BGSProjectile>    (0x816, Config::LeviathanModESP);
    Levi->LeviProjBaseA     = dataHandler->LookupForm<RE::BGSProjectile>    (0x822, Config::LeviathanModESP);
    Levi->SpellLeviProjL    = dataHandler->LookupForm<RE::SpellItem>        (0x811, Config::LeviathanModESP);
    Levi->SpellLeviProjH    = dataHandler->LookupForm<RE::SpellItem>        (0x813, Config::LeviathanModESP);
    Levi->SpellCatchLevi    = dataHandler->LookupForm<RE::SpellItem>        (0x81D, Config::LeviathanModESP);
    Levi->SpellLeviProjA    = dataHandler->LookupForm<RE::SpellItem>        (0x823, Config::LeviathanModESP);
    Levi->EnchCharge        = dataHandler->LookupForm<RE::EnchantmentItem>  (0x45D58, "Skyrim.esm");    //  EnchWeaponFrostDamage06, +30 frost damage, ff, touch
    if (Levi->LeviProjBaseL && Levi->LeviProjBaseH && Levi->LeviProjBaseA)
            spdlog::debug("Leviathan Axe projectiles are {}, {} and {}", Levi->LeviProjBaseL->GetName(), Levi->LeviProjBaseH->GetName(), Levi->LeviProjBaseA->GetName());
    else    spdlog::warn("Can't find Leviathan Axe projectiles");
    if (Levi->SpellLeviProjL && Levi->SpellLeviProjH)   
            spdlog::debug("Leviathan Axe projectile spells are {} and {}", Levi->SpellLeviProjL->GetName(), Levi->SpellLeviProjH->GetName());
    else    spdlog::warn("Can't find Leviathan Axe projectile spells");
    if (Levi->SpellCatchLevi) {
            Levi->EffCatchLevi = Levi->SpellCatchLevi->effects[0]->baseEffect;
            spdlog::debug("Leviathan Axe catching spell is {}", Levi->SpellCatchLevi->GetName()); Levi->EffCatchLevi = Levi->SpellCatchLevi->effects[0]->baseEffect;}
    else    spdlog::warn("Can't find Leviathan Axe catching spell");
    if (Levi->SpellLeviProjA)
            spdlog::debug("Leviathan Axe calling spell is {}", Levi->SpellLeviProjA->GetName());
    else    spdlog::warn("Can't find Leviathan Axe calling spell");
    if (Levi->EnchCharge) {         
            spdlog::debug("Default Leviathan Axe charging enchantment is {}", Levi->EnchCharge->GetName());}
    else    spdlog::warn("Can't find default Leviathan Axe enchantment spell");

    //  The Draupnir Spear's stuff
      Draupnir::DraupnirSpearProjBaseL  = dataHandler->LookupForm<RE::BGSProjectile>(0x802, Config::DraupnirModESP);
      Draupnir::DraupnirsCallProjBaseL  = dataHandler->LookupForm<RE::BGSProjectile>(0x818, Config::DraupnirModESP);
    Draupnir::SpellDraupnirProjL        = dataHandler->LookupForm<RE::SpellItem>    (0x800, Config::DraupnirModESP);
    Draupnir::SpellDraupnirsCallProjL   = dataHandler->LookupForm<RE::SpellItem>    (0x805, Config::DraupnirModESP);
    Draupnir::DraupnirExplosion         = dataHandler->LookupForm<RE::BGSExplosion> (0x809, Config::DraupnirModESP);
    if (Draupnir::DraupnirSpearProjBaseL)
            spdlog::debug("Draupnir Spear projectile is {}", Draupnir::DraupnirSpearProjBaseL->GetName());
    else    spdlog::warn("Can't find Draupnir Spear projectile");
    if (Draupnir::DraupnirsCallProjBaseL)
            spdlog::debug("Draupnir's call projectile is {}", Draupnir::DraupnirsCallProjBaseL->GetName());
    else    spdlog::warn("Can't find Draupnir Spear projectile");
    if (Draupnir::SpellDraupnirProjL)
            spdlog::debug("Draupnir Spear projectile spell is {}", Draupnir::SpellDraupnirProjL->GetName());
    else    spdlog::warn("Can't find Draupnir Spear projectile spell");
    if (Draupnir::SpellDraupnirsCallProjL)
            spdlog::debug("Draupnir's call spell is {}", Draupnir::SpellDraupnirsCallProjL->GetName());
    else    spdlog::warn("Can't find Draupnir Spear projectile spell");
    if (Draupnir::DraupnirExplosion)
            spdlog::debug("Draupnir Spear explosion is {}", Draupnir::DraupnirExplosion->GetName());
    else    spdlog::warn("Can't find Draupnir Spear explosion");

    auto kratos = Kratos::GetSingleton();
    kratos->SpellAxeThrownState       = dataHandler->LookupForm<RE::SpellItem>(0x81B, Config::LeviathanModESP);
    kratos->SpellSpartanRage        = dataHandler->LookupForm<RE::SpellItem>(0x80F, Config::LeviathanModESP);
    kratos->SpellRageCoolDown       = dataHandler->LookupForm<RE::SpellItem>(0x829, Config::LeviathanModESP);
    kratos->SpellLeviChargeCD       = dataHandler->LookupForm<RE::SpellItem>(0x827, Config::LeviathanModESP);
    kratos->SpellAxeCallButton      = dataHandler->LookupForm<RE::SpellItem>(0x803, Config::LeviathanModESP);
    kratos->SpellAimButton          = dataHandler->LookupForm<RE::SpellItem>(0x805, Config::LeviathanModESP);
    kratos->SpellRunicButton        = dataHandler->LookupForm<RE::SpellItem>(0x809, Config::LeviathanModESP);
    kratos->SpellFinisherButton     = dataHandler->LookupForm<RE::SpellItem>(0x807, Config::LeviathanModESP);
    kratos->SpellMidDistButton      = dataHandler->LookupForm<RE::SpellItem>(0x80B, Config::LeviathanModESP);
    kratos->SpellLongDistButton     = dataHandler->LookupForm<RE::SpellItem>(0x80D, Config::LeviathanModESP);
    kratos->soundEffect.catchLevi   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x2398A, "Skyrim.esm");
    kratos->soundEffect.callLevi    = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x7D013, "Skyrim.esm");
    kratos->soundEffect.chargeLevi  = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EDD5, "Skyrim.esm");
    kratos->soundEffect.chargeLeviEnd   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EAC6, "Skyrim.esm");
    kratos->VFXeffect.handFrost         = dataHandler->LookupForm<RE::BGSArtObject>(0x42854, "Skyrim.esm");
    kratos->VFXeffect.handFrostBright   = dataHandler->LookupForm<RE::BGSArtObject>(0x334B9, "Skyrim.esm");
    kratos->VFXeffect.handFlame     = dataHandler->LookupForm<RE::BGSArtObject>(0x1B211, "Skyrim.esm");
    kratos->VFXeffect.iceCloak      = dataHandler->LookupForm<RE::BGSArtObject>(0x4253F, "Skyrim.esm");
    kratos->VFXeffect.fireCloak     = dataHandler->LookupForm<RE::BGSArtObject>(0x2ACD7, "Skyrim.esm");
    kratos->action.normalAttack     = dataHandler->LookupForm<RE::BGSAction>(0x13005, "Skyrim.esm");
    kratos->action.powerAttack      = dataHandler->LookupForm<RE::BGSAction>(0x13383, "Skyrim.esm");

    if (!kratos->SpellAxeThrownState || !kratos->SpellSpartanRage || !kratos->SpellRageCoolDown || !kratos->SpellLeviChargeCD || !kratos->VFXeffect.handFrost)
        spdlog::error("Check Kratos's spell addresses");

    LeviathanAxeKWD     = dataHandler->LookupForm<RE::BGSKeyword>(0x817, Config::LeviathanModESP);
    BladeOfChaosKWD     = dataHandler->LookupForm<RE::BGSKeyword>(0x818, Config::LeviathanModESP);
    DraupnirSpearKWD    = dataHandler->LookupForm<RE::BGSKeyword>(0x819, Config::LeviathanModESP);
    BladeOfOlympusKWD   = dataHandler->LookupForm<RE::BGSKeyword>(0x82B, Config::LeviathanModESP);
    GuardianShieldKWD   = dataHandler->LookupForm<RE::BGSKeyword>(0x81A, Config::LeviathanModESP);
}
/**/
void APIs::Request()
{
        spdlog::info("API's loading...");
//  if (!tdm) {
//      tdm = reinterpret_cast<TDM_API::IVTDM1*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V1));
//      if (tdm) spdlog::debug("TDM API loaded");
//      else spdlog::warn("TDM API failed to load");
//  }
}