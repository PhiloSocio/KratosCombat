#include "logger.h"
#include "Papyrus.h"
#include "events.h"
#include "Assets.h"
#include "hook.h"

#ifdef KRATOS_COMBAT_3
    #include "RelicManager.h"

    static constexpr std::uint32_t kSerializationUniqueID = 'KRTS';

    void OnSerializationSave(SKSE::SerializationInterface* a_intfc)
    {
        spdlog::info("Serialization: save");
        RelicManager::GetSingleton()->OnSaveGame(a_intfc);
    }

    void OnSerializationLoad(SKSE::SerializationInterface* a_intfc)
    {
        spdlog::info("Serialization: load");
        RelicManager::GetSingleton()->OnPostLoadGame(a_intfc);
    }

    void OnSerializationRevert(SKSE::SerializationInterface* a_intfc)
    {
        spdlog::info("Serialization: revert");
        RelicManager::GetSingleton()->OnRevert();
    }
#else
    #include "MainKratosCombat.h"
#endif

inline bool UpdateConfig() 
{
    if (!Config::CheckForms()) {
        spdlog::warn("can't get mandatory forms! check the required esp files.");
    //  Config::CheckConfig(true);
        Config::CheckConfig();
    } else {
    //  Config::CheckConfig(true);
        Config::CheckConfig();
        return true;
    }
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
        if (UpdateConfig() && Assets::GetSingleton()->Initialize())
            InstallHooks();
        break;
    case SKSE::MessagingInterface::kPostLoad:
        if (APIs::Request())
            Config::IsPrecisionInstalled = true;
        break;
    case SKSE::MessagingInterface::kPreLoadGame:

#ifdef KRATOS_COMBAT_3
        RelicManager::GetSingleton()->OnPreLoadGame();
#else
        if (auto Levi = LeviathanAxe::GetSingleton(); auto mjolnir = Mjolnir::GetSingleton()) {
            Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, false, true);
            mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag, false, true);
            spdlog::info("charged weapons reset because loading the game!");
            Levi->trailUpdate.Done();
            mjolnir->trailUpdate.Done();
        }
#endif
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
        if (!Config::CheckForms()) spdlog::warn("can't get important magic effects! Check the esp files!");
        else if (RegisterEvents()) {
            Papyrus::eventsRegistered = true;

#ifdef KRATOS_COMBAT_3
        //    RelicManager::GetSingleton()->OnPostLoadGame();
#else
            WeaponIdentify::Initialize();
            WeaponIdentify::WeaponCheck();
#endif
        }
        break;
    case SKSE::MessagingInterface::kNewGame:
        if (!Config::CheckForms()) spdlog::warn("can't get important magic effects! Check the esp files!");
        else if (RegisterEvents()) {
            Papyrus::eventsRegistered = true;

#ifdef KRATOS_COMBAT_3
        RelicManager::GetSingleton()->OnRevert();
        //    RelicManager::GetSingleton()->OnPostLoadGame();
#else
            WeaponIdentify::Initialize();
            WeaponIdentify::WeaponCheck();
#endif
        }
        break;
    case SKSE::MessagingInterface::kSaveGame:

#ifdef KRATOS_COMBAT_3
        RelicManager::GetSingleton()->OnSaveGame();
#else
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
#endif

        break;
    }
}

SKSEPluginInfo(SKSE::PluginDeclaration::PluginDeclarationInfo{
        .Version = { 2, 0, 8, 9 },
        .Name = "KratosCombat",
        .Author = "AnArchos",
        .SupportEmail = "patreon.com/AnArchos",
        .StructCompatibility = ::SKSE::StructCompatibility::Independent,
        .RuntimeCompatibility = ::SKSE::VersionIndependence::AddressLibrary,
        .MinimumSKSEVersion = { 2, 0, 0, 2 }
    }
);
SKSE_PLUGIN_LOAD(const SKSE::LoadInterface *skse)
{
    REL::Module::reset();

    SetupLog();

    auto* plugin  = SKSE::PluginDeclaration::GetSingleton();
    spdlog::info("{} v{} is loading...", plugin->GetName(), plugin->GetVersion());

    SKSE::Init(skse);
    SKSE::AllocTrampoline(1 << 10);

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", MessageHandler)) {
        return false;
    }

    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(kSerializationUniqueID);
    serialization->SetSaveCallback(OnSerializationSave);
    serialization->SetLoadCallback(OnSerializationLoad);
    serialization->SetRevertCallback(OnSerializationRevert);

    spdlog::info("{} by {} has finished loading. Support for more mods! {}", plugin->GetName(), plugin->GetAuthor(), plugin->GetSupportEmail());

    return true;
}