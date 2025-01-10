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
    //  APIs::Request();
        break;
    case SKSE::MessagingInterface::kPreLoadGame:
        if (auto Levi = LeviathanAxe::GetSingleton(); Levi) {
            Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, false, true);
            spdlog::debug("last charged weapons checked!");
        }
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
    case SKSE::MessagingInterface::kNewGame:
        if (!Kratos::GetSingleton()->Initialize()) spdlog::warn("can't get important magic effects!");
        else if (RegisterEvents()) {Papyrus::eventsRegistered = true; WeaponIdentify::Initialize(); WeaponIdentify::WeaponCheck(false);}
        break;
    case SKSE::MessagingInterface::kSaveGame:
        if (auto Levi = LeviathanAxe::GetSingleton(); Levi) {
            Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, false, true);
            spdlog::debug("last charged weapons checked!");
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
//  ProjectileHook::runtimeVer = skse->RuntimeVersion();


    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", MessageHandler)) {
        return false;
    }

    spdlog::info("{} by {} has finished loading. Support for more mods! {}", plugin->GetName(), plugin->GetAuthor(), plugin->GetSupportEmail());

    return true;
}
/**/
SKSEPluginInfo(
    .Version = REL::Version{ 1, 9, 0, 0 },
    .Name = "KratosCombat"sv,
    .Author = "AnArchos"sv,
    .SupportEmail = "patreon.com/AnArchos"sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
    .MinimumSKSEVersion = REL::Version{ 2, 0, 0, 2 }
)
