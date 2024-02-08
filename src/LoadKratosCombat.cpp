#include "logger.h"
#include "MainKratosCombat.h"

#define ConfigPath "Data\\SKSE\\Plugins\\KratosCombat.ini"

inline void UpdateConfig() 
{
	Config::CheckConfig();
	Config::CheckProjectiles();
}
inline void InstallHooks() 
{
	ProjectileHook::Hook();
}
inline bool RegisterEvents() 
{
	return
		AnimationEventTracker::Register();
	//	InputEventTracker::Register();
}
void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		InstallHooks();
		UpdateConfig();
		break;
	case SKSE::MessagingInterface::kPostLoad:
	//	APIs::Request();
		break;
	case SKSE::MessagingInterface::kPreLoadGame:
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
	case SKSE::MessagingInterface::kNewGame:
		if (RegisterEvents()) WeaponIdentify::WeaponCheck();
		break;
	}
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

	SetupLog();

    auto* plugin  = SKSE::PluginDeclaration::GetSingleton();
    spdlog::info("{} v{} is loading...", plugin->GetName(), plugin->GetVersion());

    SKSE::Init(skse);
	SKSE::AllocTrampoline(1 << 10);
//	ProjectileHook::runtimeVer = skse->RuntimeVersion();


    auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}

    spdlog::info("{} by {} has finished loading. Support for more mods! {}", plugin->GetName(), plugin->GetAuthor(), plugin->GetSupportEmail());

    return true;
}
/**/
SKSEPluginInfo(
    .Version = REL::Version{ 1, 0, 1, 0 },
    .Name = "KratosCombat"sv,
    .Author = "AnArchos"sv,
    .SupportEmail = "patreon.com/AnArchos"sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
    .MinimumSKSEVersion = REL::Version{ 2, 0, 0, 2 }
)
