#include "logger.h"
#include "Papyrus.h"
#include "MainKratosCombat.h"

inline bool UpdateConfig() 
{
//  Config::CheckConfig(true);
    Config::CheckConfig();
    if (!Config::CheckForms()) spdlog::warn("can't get mandatory forms! check the required esp files.");
    else return true;
    return false;
}
inline void InstallHooks() 
{
    ProjectileHook::Hook();
    PlayerHook::Hook();
    AttackHook::Hook();
}
void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        Papyrus::Register();
        if (UpdateConfig())
            InstallHooks();
        break;
    case SKSE::MessagingInterface::kPostLoad:
        APIs::Request();
        break;
    case SKSE::MessagingInterface::kPreLoadGame:
        if (auto Levi = LeviathanAxe::GetSingleton(); auto mjolnir = Mjolnir::GetSingleton()) {
            Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, false, true);
            mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag, false, true);
            spdlog::info("charged weapons reset because loading the game!");
            Levi->trailUpdate.Done();
            mjolnir->trailUpdate.Done();
        }
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
    case SKSE::MessagingInterface::kNewGame:
        if (!Kratos::GetSingleton()->Initialize()) spdlog::warn("can't get important magic effects! Check the esp files!");
        else if (RegisterEvents()) {Papyrus::eventsRegistered = true; WeaponIdentify::Initialize(); WeaponIdentify::WeaponCheck();}
        break;
    case SKSE::MessagingInterface::kSaveGame:
        if (auto Levi = LeviathanAxe::GetSingleton(); auto mjolnir = Mjolnir::GetSingleton()) {
            Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, false, true);
            mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag, false, true);
            spdlog::info("charged weapons reset because saving the game!");
        }
        if (auto playerCharacter = RE::PlayerCharacter::GetSingleton(); playerCharacter) {
            playerCharacter->SetGraphVariableBool("SkipEquipAnimation", _skipEquipAnim);    //  Reset to default values
            playerCharacter->SetGraphVariableInt("LoadBoundObjectDelay", _load3Ddelay);     //  Reset to default values
            playerCharacter->SetGraphVariableBool("Skip3DLoading", _skipLoad3D);            //  Reset to default values
        }
        break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    SetupLog();

    auto* plugin  = SKSE::PluginDeclaration::GetSingleton();
    spdlog::info("{} v{} is loading...", plugin->GetName(), plugin->GetVersion());

    SKSE::Init(skse);
    SKSE::AllocTrampoline(1 << 10);

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", MessageHandler)) {
        return false;
    }

    spdlog::info("{} by {} has finished loading. Support for more mods! {}", plugin->GetName(), plugin->GetAuthor(), plugin->GetSupportEmail());

    return true;
}
/**/
SKSEPluginInfo(
    .Version = REL::Version{ 2, 0, 8, 0 },
    .Name = "KratosCombat"sv,
    .Author = "AnArchos"sv,
    .SupportEmail = "patreon.com/AnArchos"sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
    .MinimumSKSEVersion = REL::Version{ 2, 0, 0, 2 }
)
