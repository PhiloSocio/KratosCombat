#include "logger.h"
#include "MainKratosCombat.h"

#define ConfigPath "Data\\SKSE\\Plugins\\KratosCombat.ini"

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
        Config::CheckConfig();
		ProjectileHook::Hook();
		break;
	case SKSE::MessagingInterface::kPostLoad:
	//	APIs::Request();
		break;
	case SKSE::MessagingInterface::kPreLoadGame:
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
		AnimationEventTracker::GetSingleton()->Register();
		InputEventTracker::GetSingleton()->Register();
		WeaponIdentify::WeaponCheck();
		Config::CheckProjectiles();
        break;
	case SKSE::MessagingInterface::kNewGame:
		break;
	}
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    SKSE::Init(skse);
	SKSE::AllocTrampoline(1 << 10);
	ProjectileHook::runtimeVer = skse->RuntimeVersion();

	SetupLog();


    auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}

	
    return true;
}
/**/
SKSEPluginInfo(
    .Version = REL::Version{ 0, 0, 1, 0 },
    .Name = "KratosCombat"sv,
    .Author = "AnArchos"sv,
    .SupportEmail = "patreon.com/AnArchos"sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
    .MinimumSKSEVersion = REL::Version{ 2, 0, 0, 2 }
)
