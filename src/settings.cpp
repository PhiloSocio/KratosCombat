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

//	ReadIntSetting(ini, "Main", "iAimButtonID", InputEventTracker::AimButton);
//	ReadIntSetting(ini, "Main", "iCallingButtonID", InputEventTracker::AxeCallButton);

	ReadFloatSetting(ini, "Main", "fThrowSpeed", ThrowSpeed);
	ReadFloatSetting(ini, "Main", "fThrowRotationSpeed", ThrowRotationSpeed);

	ReadFloatSetting(ini, "Main", "fArrivalTime", ArrivalTime);
	ReadFloatSetting(ini, "Main", "fArrivalRotationSpeed", ArrivalRotationSpeed);
	ReadFloatSetting(ini, "Main", "fArrivalRoadCurveMagnitude", ArrivalRoadCurveMagnitude);
	ReadFloatSetting(ini, "Main", "fCatchingTreshold", CatchingTreshold);

	ReadFloatSetting(ini, "Main", "fMaxArrivalSpeed", MaxArrivalSpeed);
	ReadFloatSetting(ini, "Main", "fMinArrivalSpeed", MinArrivalSpeed);
	ReadFloatSetting(ini, "Main", "fMaxAxeStuckAngle", MaxAxeStuckAngle);
	ReadFloatSetting(ini, "Main", "fMinAxeStuckAngle", MinAxeStuckAngle);

	ReadIntSetting(ini, "Main", "iDraupnirExplodableCount", DraupnirSpearCount);
	ReadFloatSetting(ini, "Main", "fProjectileCastingDelay", ProjCastDelay);
	
	ReadStringSetting(ini, "Main", "sLeviathanModESP", LeviathanModESP);
	ReadStringSetting(ini, "Main", "sDraupnirModESP", DraupnirModESP);

	ReadStringSetting(ini, "Main", "sWeaponThrowingEvent", ThrowEvent);
	ReadStringSetting(ini, "Main", "sWeaponCallingEvent", CallEvent);
	ReadStringSetting(ini, "Main", "sAxeChargeEvent", ChargeLeviEvent);
	ReadStringSetting(ini, "Main", "sDraupnirsCallEvent", DraupnirsCallEvent);

	ReadStringSetting(ini, "Main", "sLeviathanAxeKeyword", LeviathanAxeKeyword);
	ReadStringSetting(ini, "Main", "sBladeOfChaosKeyword", BladeOfChaosKeyword);
	ReadStringSetting(ini, "Main", "sDraupnirSpearKeyword", DraupnirSpearKeyword);

	if (ini.GetBoolValue("Main", "bDontHitWhileArriving")) DontDamageWhileArrive = true;
	else DontDamageWhileArrive = false;

	if		(ThrowSpeed < 1000.f)	ThrowSpeed = 1000.f;
	else if	(ThrowSpeed > 100000.f)	ThrowSpeed = 100000.f;

	if		(ArrivalTime <= 0.f)	ArrivalTime = 0.02f;
	else if	(ArrivalTime > 360.f)	ArrivalTime = 360.f;

	if		(DraupnirSpearCount > 9)	DraupnirSpearCount = 9;
	else if	(DraupnirSpearCount < 1)	DraupnirSpearCount = 1;
	DraupnirSpearCount -= 1;				// DraupnirSpearCount starts with 0

	for (int i = Config::DraupnirSpearCount; i <= 9; i++) {
		if (Draupnir::DraupnirSpearProjectiles[i] != nullptr) {
			Draupnir::DraupnirSpearProjectiles[i] = nullptr;}
		if (Draupnir::DraupnirSpearHitBones[i] != nullptr) {
			Draupnir::DraupnirSpearHitBones[i] = nullptr;}
			spdlog::debug("Draupnir spear {} is nullptr", i);
	}

	ArrivalRoadCurveMagnitude *= 0.017453292f;
	ArrivalRoadCurveMagnitude = sinf(ArrivalRoadCurveMagnitude);
	ThrowRotationSpeed *= 0.017453292f;
	ArrivalRotationSpeed *= 0.017453292f;
	MinAxeStuckAngle *= 0.017453292f;
	MaxAxeStuckAngle *= 0.017453292f;

	if (ini.GetBoolValue("Main", "DebugModeOpen")) {
		spdlog::set_level(spdlog::level::debug);
		spdlog::debug("Debug mode enabled");
	} else spdlog::set_level(spdlog::level::info);
	spdlog::info("Kratos Combat's configurations checked.");
}

void Config::CheckProjectiles()
{
	auto dataHandler = RE::TESDataHandler::GetSingleton();
	Leviathan::LeviProjBaseL 	= dataHandler->LookupForm<RE::BGSProjectile>	(0x815, Config::LeviathanModESP);
  	Leviathan::LeviProjBaseH	= dataHandler->LookupForm<RE::BGSProjectile>	(0x816, Config::LeviathanModESP);
  	Leviathan::LeviProjBaseA	= dataHandler->LookupForm<RE::BGSProjectile>	(0x822, Config::LeviathanModESP);
	Leviathan::SpellLeviProjL 	= dataHandler->LookupForm<RE::SpellItem>		(0x811, Config::LeviathanModESP);
  	Leviathan::SpellLeviProjH	= dataHandler->LookupForm<RE::SpellItem>		(0x813, Config::LeviathanModESP);
	Leviathan::SpellCatchLevi 	= dataHandler->LookupForm<RE::SpellItem>		(0x81D, Config::LeviathanModESP);
  	Leviathan::SpellLeviProjA	= dataHandler->LookupForm<RE::SpellItem>		(0x823, Config::LeviathanModESP);
	Leviathan::EnchCharge 		= dataHandler->LookupForm<RE::EnchantmentItem>	(0x45D58, "Skyrim.esm");	//	EnchWeaponFrostDamage06, +30 frost damage, ff, touch
	if (Leviathan::LeviProjBaseL && Leviathan::LeviProjBaseH && Leviathan::LeviProjBaseA)
			spdlog::debug("Leviathan Axe projectiles are {}, {} and {}", Leviathan::LeviProjBaseL->GetName(), Leviathan::LeviProjBaseH->GetName(), Leviathan::LeviProjBaseA->GetName());
	else 	spdlog::warn("Can't find Leviathan Axe projectiles");
	if (Leviathan::SpellLeviProjL && Leviathan::SpellLeviProjH)	
			spdlog::debug("Leviathan Axe projectile spells are {} and {}", Leviathan::SpellLeviProjL->GetName(), Leviathan::SpellLeviProjH->GetName());
	else 	spdlog::warn("Can't find Leviathan Axe projectile spells");
	if (Leviathan::SpellCatchLevi) {
		Leviathan::EffCatchLevi = Leviathan::SpellCatchLevi->effects[0]->baseEffect;
			spdlog::debug("Leviathan Axe catching spell is {}", Leviathan::SpellCatchLevi->GetName()); Leviathan::EffCatchLevi = Leviathan::SpellCatchLevi->effects[0]->baseEffect;}
	else 	spdlog::warn("Can't find Leviathan Axe catching spell");
	if (Leviathan::SpellLeviProjA)
			spdlog::debug("Leviathan Axe calling spell is {}", Leviathan::SpellLeviProjA->GetName());
	else 	spdlog::warn("Can't find Leviathan Axe calling spell");
	if (Leviathan::EnchCharge) {			
			spdlog::debug("Default Leviathan Axe charging enchantment is {}", Leviathan::EnchCharge->GetName());}
	else 	spdlog::warn("Can't find default Leviathan Axe enchantment spell");

  	Draupnir::DraupnirSpearProjBaseL	= dataHandler->LookupForm<RE::BGSProjectile>(0x802, Config::DraupnirModESP);
  	Draupnir::DraupnirsCallProjBaseL	= dataHandler->LookupForm<RE::BGSProjectile>(0x818, Config::DraupnirModESP);
	Draupnir::SpellDraupnirProjL 		= dataHandler->LookupForm<RE::SpellItem>	(0x800, Config::DraupnirModESP);
	Draupnir::SpellDraupnirsCallProjL	= dataHandler->LookupForm<RE::SpellItem>	(0x805, Config::DraupnirModESP);
	Draupnir::DraupnirExplosion 		= dataHandler->LookupForm<RE::BGSExplosion>	(0x809, Config::DraupnirModESP);
	if (Draupnir::DraupnirSpearProjBaseL)
			spdlog::debug("Draupnir Spear projectile is {}", Draupnir::DraupnirSpearProjBaseL->GetName());
	else 	spdlog::warn("Can't find Draupnir Spear projectile");
	if (Draupnir::DraupnirsCallProjBaseL)
			spdlog::debug("Draupnir's call projectile is {}", Draupnir::DraupnirsCallProjBaseL->GetName());
	else 	spdlog::warn("Can't find Draupnir Spear projectile");
	if (Draupnir::SpellDraupnirProjL)
			spdlog::debug("Draupnir Spear projectile spell is {}", Draupnir::SpellDraupnirProjL->GetName());
	else 	spdlog::warn("Can't find Draupnir Spear projectile spell");
	if (Draupnir::SpellDraupnirsCallProjL)
			spdlog::debug("Draupnir's call spell is {}", Draupnir::SpellDraupnirsCallProjL->GetName());
	else 	spdlog::warn("Can't find Draupnir Spear projectile spell");
	if (Draupnir::DraupnirExplosion)
			spdlog::debug("Draupnir Spear explosion is {}", Draupnir::DraupnirExplosion->GetName());
	else 	spdlog::warn("Can't find Draupnir Spear explosion");

  	Config::SpellBHstate		= dataHandler->LookupForm<RE::SpellItem>(0x81B, Config::LeviathanModESP);

	SpellCastTracker::SpellAxeCallButton	= dataHandler->LookupForm<RE::SpellItem>	(0x803, Config::LeviathanModESP);
	SpellCastTracker::SpellAimButton 		= dataHandler->LookupForm<RE::SpellItem>	(0x805, Config::LeviathanModESP);
	if (SpellCastTracker::SpellAxeCallButton) {
		SpellCastTracker::EffectAxeCallButton = SpellCastTracker::SpellAxeCallButton->effects[0]->baseEffect;
			spdlog::debug("Axe call button spell is {}", SpellCastTracker::SpellAxeCallButton->GetName());
	} else 	spdlog::warn("Can't find Axe call button spell");
	if (SpellCastTracker::SpellAimButton) {
		SpellCastTracker::EffectAimButton = SpellCastTracker::SpellAimButton->effects[0]->baseEffect;
			spdlog::debug("Aim button spell is {}", SpellCastTracker::SpellAimButton->GetName());
	} else 	spdlog::warn("Can't find Aim button spell");
}
/**/
void APIs::Request()
{
		spdlog::info("API's loading...");
//	if (!tdm) {
//		tdm = reinterpret_cast<TDM_API::IVTDM1*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V1));
//		if (tdm) spdlog::debug("TDM API loaded");
//		else spdlog::warn("TDM API failed to load");
//	}
}