#include "util.h"
#include "MainKratosCombat.h"
//#include "API/TrueDirectionalMovementAPI.h"

using namespace Util;
//static std::jthread axeEquipThread;
//static std::jthread axeChargeThread;
static void DelayedCast(RE::Actor* a_caster, RE::SpellItem* a_spell, RE::NiNode* a_target, float a_delaySec)
{
	if (!a_spell) return;
	if (!a_caster) return;
	int delay = static_cast<int>(a_delaySec * 1000.f);
	spdlog::debug("draupnir's call start...");
	std::this_thread::sleep_for(std::chrono::milliseconds(delay));
	if (!a_spell) return;
	if (!a_caster) return;
	spdlog::debug("draupnir's call triggered!");
//	a_caster->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(a_spell, false, nullptr, 1.f, false, 10.f, a_caster);
	
	if (!a_target) return;
	RE::ProjectileHandle pHandle;
	RE::NiPoint3 startPoint = a_target->world.translate + RE::NiPoint3(0.f, 0.f, 128.f);
	RE::Projectile::ProjectileRot pRot = {a_caster->GetAimAngle(), a_caster->GetAimHeading()};
	RE::Projectile::LaunchSpell(&pHandle, a_caster, a_spell, startPoint, pRot);
	spdlog::debug("draupnir's call end!");
}

//WEAPON IDENTIFICATION
void WeaponIdentify::WeaponIdentifier(RE::PlayerCharacter* a_player, RE::TESObjectWEAP* a_RHandWeapon)
{
	if (!a_RHandWeapon) return;
    auto RHandWeaponKWDlist = a_RHandWeapon->As<BGSKeywordForm>();
    auto RelicName = "not a Relic";
//	auto address = reinterpret_cast<std::uintptr_t>(a_RHandWeapon);

    if (RHandWeaponKWDlist->HasKeywordString(Config::LeviathanAxeKeyword)) {
    	isLeviathanAxe = true;
		isRelic = true;
    	RelicName = "the Leviathan Axe";
		LeviathanAxe = a_RHandWeapon;
    	spdlog::info("{} is {}", LeviathanAxe->GetName(), RelicName);

		if (WeaponIdentify::LeviathanAxe->HasWorldModel()) {spdlog::debug("Levi is throwable"); Leviathan::SetThrowState(Leviathan::ThrowState::kThrowable);}
		else spdlog::debug("Levi is not equipped for real");

		if (Leviathan::SpellCatchLevi && a_player->AsMagicTarget()->HasMagicEffect(Leviathan::EffCatchLevi)) {a_player->RemoveSpell(Leviathan::SpellCatchLevi);}

		if (Leviathan::LeviathanAxeProjectileA 
		&& (Leviathan::throwState == Leviathan::ThrowState::kArriving
		 || Leviathan::throwState == Leviathan::ThrowState::kArrived)) 
		  	Leviathan::Catch(Leviathan::LeviathanAxeProjectileA, a_player, true);

			return;
	}
    if (RHandWeaponKWDlist->HasKeywordString(Config::BladeOfChaosKeyword)) {
    	isBladeOfChaos = true;
		isRelic = true;
    	RelicName = "the Blade of Chaos";
		BladeOfChaos = a_RHandWeapon;
//			return;
	}
    if (RHandWeaponKWDlist->HasKeywordString(Config::DraupnirSpearKeyword)) {
    	isDraupnirSpear = true;
		isRelic = true;
    	RelicName = "the Draupnir Spear";
		DraupnirSpear = a_RHandWeapon;
//			return;
	}
    spdlog::info("{} is {}", a_RHandWeapon->GetName(), RelicName);
}
void WeaponIdentify::WeaponCheck()
{
    isLeviathanAxe = false;
    isBladeOfChaos = false;
    isDraupnirSpear = false;
	isRelic = false;

	const auto AnArchos = PlayerCharacter::GetSingleton();
	if		(!AnArchos) return;
	if		(AnArchos->GetNodeByName("NPC R Finger20 [RF20]"))	{RHandBone = AnArchos->GetNodeByName("NPC R Finger20 [RF20]");}
	else if	(AnArchos->GetNodeByName("Weapon"))					{RHandBone = AnArchos->GetNodeByName("Weapon");}
	else if	(AnArchos->GetNodeByName("NPC R MagicNode [RMag]"))	{RHandBone = AnArchos->GetNodeByName("NPC R MagicNode [RMag]");}
	else if	(AnArchos->GetNodeByName("NPC R Hand [RHnd]"))		{RHandBone = AnArchos->GetNodeByName("NPC R Hand [RHnd]");}

	if	(AnArchos->GetNodeByName("Weapon"))						{WeaponBone = AnArchos->GetNodeByName("Weapon");}
	//	spdlog::debug("Right hand bone is {}", RHandBone->name);

	auto pcSkillArchery	= AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kArchery);
	auto pcSkill1Handed	= AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kOneHanded);
	auto pcDamageMult	= AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAttackDamageMult);
	DamageMult	= 1.f + (pcSkill1Handed / 120) + (pcSkillArchery / 80);
	DamageMult	*= pcDamageMult;

	if (AnArchos->GetEquippedObject(false)) {
		if (AnArchos->GetEquippedObject(false)->IsWeapon()){
			auto RHandWeapon = AnArchos->GetEquippedObject(false)->As<TESObjectWEAP>();
				return WeaponIdentifier(AnArchos, RHandWeapon);
		}
	}
	spdlog::info("you are barehanded");
}
bool WeaponIdentify::IsRelic(RE::BGSProjectile *a_baseProj, bool a_onlyLevi, bool a_onlyDraupnir)
{
	if (!a_baseProj) return false;
	auto leviA = Leviathan::LeviProjBaseA;
	auto leviL = Leviathan::LeviProjBaseL;
	auto leviH = Leviathan::LeviProjBaseH;
	auto draup = Draupnir::DraupnirSpearProjBaseL;
	if (a_onlyLevi) return (a_baseProj == leviA || a_baseProj == leviL || a_baseProj == leviH);
	else if (a_onlyDraupnir) return (a_baseProj == draup);

	return (a_baseProj == leviA || a_baseProj == leviL || a_baseProj == leviH || a_baseProj == draup);
}
//LEVIATHAN AXE
Leviathan::ThrowState Leviathan::GetThrowState() 	{return throwState;}
void Leviathan::SetThrowState(ThrowState a_state)	{throwState = a_state;
	return;
	auto stateName = "none";
	switch (throwState) {
		case ThrowState::kNone:
			break;
		case ThrowState::kThrowable:
		stateName = "throwable";
			break;
		case ThrowState::kThrown:
		stateName = "thrown";
			break;
		case ThrowState::kCanArrive:
		stateName = "can arrive";
			break;
		case ThrowState::kArriving:
		stateName = "arriving";
			break;
		case ThrowState::kArrived:
		stateName = "arrived";
			break;
	}
	spdlog::debug("!!Throw state changed as {}", stateName);
}
/*
static void ReLaunch(RE::Projectile* a_this, RE::NiPoint3* a_dV)
{
	using func_t = decltype(&ReLaunch);
	REL::Relocation<func_t> func{ RELOCATION_ID(43007, 44198) };
	return func(a_this, a_dV);
}
*/
inline void Leviathan::SetStartPos(RE::NiPoint3& a_point, RE::PlayerCharacter* a_caller)
{
	auto stuckedLevi =	LastLeviProjectile ? LastLeviProjectile : nullptr;
	if (!stuckedLevi) stuckedLevi =	(LeviathanAxeProjectileL ? LeviathanAxeProjectileL : 
									(LeviathanAxeProjectileH ? LeviathanAxeProjectileH : nullptr));
	if (stuckedLevi) {
		a_point = stuckedLevi->data.location; 
	} else spdlog::debug("we cant get leviathan's stucked proj!");

//	if (LeviathanAxeProjectileA) {
//		auto& rtData = stuckedLevi->GetProjectileRuntimeData();
//		rtData.flags |= (1 << 25);
//	}

	if (leviStuckedBone) {
		a_point = leviStuckedBone->world.translate;
		leviStuckedBone = nullptr;
	} else spdlog::debug("levi not stucked any bone");

	if (leviStuckedActor) {
		leviStuckedActor->RemoveExtraArrows3D();
		spdlog::debug("levi stucked actor's extra arrows removed");
		leviStuckedActor = nullptr;
	//	RE::BaseExtraList extraList;
	//	RE::ExtraAttachedArrows3D* attachedArrow;
	//	auto targetData = leviStuckedActor->extraList.Remove<RE::ExtraAttachedArrows3D>(attachedArrow);
	//	for (auto attachedLevi : attachedArrow->data) {
	//		if (WeaponIdentify::IsRelic(attachedLevi.source)) {
	//			attachedLevi.arrow3D.get()->DeleteThis();
	//		}
	//	}
	} else spdlog::debug("levi not stucked anybody");

	auto pcCell = a_caller->parentCell;
	auto bound = pcCell->GetRuntimeData().cellData.exterior->worldX;
	auto pcPos = a_caller->GetPosition();
	float dist = pcPos.GetDistance(a_point);
	if (dist > 36000.f) {	// ~42000 is limit
		spdlog::info("levi too far from you! ({} m)", dist / 100.f);
		auto dir = a_point - pcPos;
		dir.Unitize();
		a_point = pcPos + dir * 36000.f;
	}
}
void Leviathan::Throw(bool isVertical)
{
	const auto AnArchos = PlayerCharacter::GetSingleton();
	const auto leviThrowSpell = (isVertical ? SpellLeviProjH : SpellLeviProjL);
//	auto leviBaseProj = (isVertical ? LeviProjBaseH : LeviProjBaseL);
	if (WeaponIdentify::LeviathanAxe && leviThrowSpell && AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe) 
	{	//switching to projectile
		const auto leviDamage = static_cast<float>(WeaponIdentify::LeviathanAxe->attackDamage);
		float mag = leviDamage * WeaponIdentify::DamageMult;
		if (isVertical) mag *= 1.5f;
		const auto leviProjEff = leviThrowSpell->effects[0];
		auto& leviProjEffSetting = leviProjEff->effectItem;
		leviProjEffSetting.magnitude = mag;

	//	if (const auto leviProjBaseEff = leviProjEff->baseEffect) {
	//		leviProjBaseEff->data.projectileBase->SetModel(WeaponIdentify::LeviathanAxe->GetModel());
	//	//	leviProjBaseEff->data.projectileBase->data.defaultWeaponSource = WeaponIdentify::LeviathanAxe;
	//	//	leviProjBaseEff->data.associatedForm = WeaponIdentify::LeviathanAxe;
	//	} else spdlog::warn("not found throwing effect!");
//
//		auto extraData = AnArchos->extraList;
//		extraData.SetInventoryChanges(RE::InventoryChanges)
	//	RE::ActorEquipManager::GetSingleton()->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, false, false, false, true, nullptr);
	//	AnArchos->RemoveWeapon(RE::BIPED_OBJECT::kOneHandAxe);
	//	AnArchos->NotifyAnimationGraph("UnequipNoAnim");
//
	//	AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(leviThrowSpell, false, nullptr, 1.f, false, mag, AnArchos);
	//	auto origin = AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->GetMagicNode()->world.translate;

		auto origin = WeaponIdentify::RHandBone->world.translate;
		RE::ProjectileHandle pHandle;
		RE::Projectile::ProjectileRot pRot = {AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
		RE::Projectile::LaunchData lData(AnArchos, origin, pRot, leviThrowSpell);

		lData.weaponSource = WeaponIdentify::LeviathanAxe;
		if (lData.weaponSource->formEnchanting) lData.enchantItem = lData.weaponSource->formEnchanting;//->data.baseEnchantment;
		else spdlog::debug("levi not has a valid enchantment");

		RE::Projectile::Launch(&pHandle, lData);

		if (!AnArchos->HasSpell(Config::SpellBHstate)) AnArchos->AddSpell(Config::SpellBHstate);
		isAxeCalled = false;
		isAxeThrowed = true;
		SetThrowState(ThrowState::kThrown);

		if (Leviathan::leviStuckedBone)		Leviathan::leviStuckedBone 	= nullptr;
		if (Leviathan::leviStuckedActor)	Leviathan::leviStuckedActor	= nullptr;
		if (Leviathan::leviLastHitActor)	Leviathan::leviLastHitActor	= nullptr;
			spdlog::info("Leviathan Axe throwed, raw damage is: {}", mag);
		if (AnArchos->HasSpell(SpellCatchLevi)) AnArchos->RemoveSpell(SpellCatchLevi);
	} else spdlog::info("Leviathan Axe is not equipped for throwing!");
}
void Leviathan::Arrive()
{
	spdlog::debug("Levi is calling...");
	auto stuckedLevi =	LastLeviProjectile ? LastLeviProjectile : nullptr;
	if (!stuckedLevi)	stuckedLevi = (LeviathanAxeProjectileL ? LeviathanAxeProjectileL : (LeviathanAxeProjectileH ? LeviathanAxeProjectileH : nullptr));
	const auto AnArchos = PlayerCharacter::GetSingleton();

	if (SpellLeviProjA) {
		isAxeCalled = true;
		isAxeThrowed = false;

		const auto leviDamage = static_cast<float>(WeaponIdentify::LeviathanAxe->attackDamage);
		float mag = leviDamage * WeaponIdentify::DamageMult;
		const auto leviProjEff = SpellLeviProjA->effects[0];
		auto& leviProjEffSetting = leviProjEff->effectItem;
		leviProjEffSetting.magnitude = mag * 0.4f;

		if (stuckedLevi) leviPosition = stuckedLevi->data.location;
		RE::NiPoint3 startPoint = leviPosition;
		SetStartPos(startPoint, AnArchos);
		RE::ProjectileHandle pHandle;
		RE::Projectile::ProjectileRot pRot = {-AnArchos->GetAimAngle(), -AnArchos->GetAimHeading()};
		RE::Projectile::LaunchData lData(AnArchos, startPoint, pRot, SpellLeviProjA);
	//	const float dist = WeaponIdentify::RHandBone->world.translate.GetDistance(startPoint);
	//	spdlog::debug("levi arriving from {}cm far away!", dist);

		if (WeaponIdentify::LeviathanAxe) {
			lData.noDamageOutsideCombat = true;	//	can be an option
			lData.weaponSource = WeaponIdentify::LeviathanAxe;
			if (lData.weaponSource->formEnchanting) lData.enchantItem = lData.weaponSource->formEnchanting;

		//	arrivalSpeed = dist / Config::ArrivalTime;
		//	if		(arrivalSpeed < Config::MinArrivalSpeed)	arrivalSpeed = Config::MinArrivalSpeed;
		//	else if	(arrivalSpeed > Config::MaxArrivalSpeed)	arrivalSpeed = Config::MaxArrivalSpeed;
		//	LeviProjBaseA->data.speed = arrivalSpeed;

			RE::Projectile::Launch(&pHandle, lData);
		//	RE::Projectile::LaunchSpell(&pHandle, AnArchos, SpellLeviProjA, startPoint, pRot);
			SetThrowState(Leviathan::ThrowState::kArriving);
		} else spdlog::warn("you don't have the axe for calling!");

/*	*/	if (stuckedLevi) {
		//	auto& fFlags = stuckedLevi->formFlags;
			auto& runtimeData = stuckedLevi->GetProjectileRuntimeData();
			auto& pFlags = runtimeData.flags;
			if (!(pFlags & (1 << 25))) {
				pFlags |= (1 << 25);
			//		spdlog::debug("levi destroyed before call");
			} else	spdlog::debug("levi is already destroyed");

		//	if ((fFlags & rFlag::kInitialized)) {
		//		if (!(fFlags & rFlag::kDisabled)) {
		//			if (!(fFlags & rFlag::kStillLoading)) {
		//				auto& runtimeData = stuckedLevi->GetProjectileRuntimeData();
		//				auto& pFlags = runtimeData.flags;
		//				if (!(pFlags & (1 << 25))) pFlags |= ~(1 << 25);
		//				spdlog::debug("levi destroyed before call");
		//				//auto expl = runtimeData.explosion;
		//				//if (expl) {
		//				//	if (pFlags & (1 << 15)) {	//	kDestroyAfterHit flag required, otherwise the explosion will cast every moment
		//				//		if ((pFlags & (1 << 31))) {
		//				//			if (pFlags & (1 << 13)) pFlags &= ~(1 << 13);
		//				//			if (pFlags & (1 << 25)) pFlags &= ~(1 << 25);
		//				//			if (runtimeData.explosionTimer > 0.f) runtimeData.explosionTimer = 0.02f;
		//				//				spdlog::debug("Levi is detached!!");
		//				//		} else {spdlog::debug("Levi is not moved (runtimeFlag)");}
		//				//	} else {spdlog::debug("Levi is not destructable (runtimeFlag)");}
		//				//} else {spdlog::info("Levi not has explosion");}
		//			} else {spdlog::debug("Levi is kStillLoading (formFlag)");}
		//		} else {spdlog::debug("Levi is kDisabled (formFlag)");}
		//	} else {spdlog::debug("Levi is not inited (formFlag)");}
		} else {spdlog::debug("Stucked Levi is nullptr!");}
	} else {spdlog::warn("WEIRD SpellLeviProjA is nullptr!");}
	spdlog::info("Levi is arriving...");
}
void Leviathan::Catch(RE::Projectile* a_levi, RE::PlayerCharacter* a_player, bool a_justDestroy)
{
	if (!a_player) return;
	if (a_justDestroy) {
		if (a_levi) {
		auto& runtimeData = a_levi->GetProjectileRuntimeData();
		runtimeData.flags |= (1 << 25);								//	set as destroyed, RE::Projectile::Flags::kDestroyed
		} return;
	}

	if (EffCatchLevi && SpellCatchLevi && !a_player->AsMagicTarget()->HasMagicEffect(EffCatchLevi)) {
		a_player->AddSpell(SpellCatchLevi);
	} else spdlog::debug("WEIRD!! you already in catch spell effect");

	if (throwState == ThrowState::kCanArrive) SetThrowState(ThrowState::kArrived);

	if (WeaponIdentify::LeviathanAxe)
		RE::ActorEquipManager::GetSingleton()->EquipObject(a_player, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, false, false, true, true);
	else spdlog::warn("you not have the leviathan axe");

	if (Leviathan::leviStuckedBone)		Leviathan::leviStuckedBone 	= nullptr;
	if (Leviathan::leviStuckedActor)	Leviathan::leviStuckedActor	= nullptr;
	if (Leviathan::leviLastHitActor)	Leviathan::leviLastHitActor	= nullptr;

	std::jthread delayedCast([=]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		isAxeCalled = false;
		if (Config::SpellBHstate && a_player->HasSpell(Config::SpellBHstate)) a_player->RemoveSpell(Config::SpellBHstate);
		spdlog::debug("Levi proj catched");
	});
	delayedCast.detach();

//	if (a_levi) {
//		auto& runtimeData = a_levi->GetProjectileRuntimeData();
//		runtimeData.flags |= (1 << 25);								//	set as destroyed, RE::Projectile::Flags::kDestroyed
//	}
}
void Leviathan::SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical)
{
	if  (a_vertical) {
		a_matrix.entry[0][0] = 0.02f;	//	const
		a_matrix.entry[0][1] = -0.54f;	//	same
		a_matrix.entry[0][2] = 0.84f;	//	
		a_matrix.entry[1][0] = 0.01f;	//	const
		a_matrix.entry[1][1] = 0.84f;	//	
		a_matrix.entry[1][2] = 0.54f;	//	
		a_matrix.entry[2][0] = -1.f;	//	const
		a_matrix.entry[2][1] = 0.0f;	//	const same
		a_matrix.entry[2][2] = 0.02f;	//	const
	} else {
		a_matrix.entry[0][0] = 0.79f;	//	
		a_matrix.entry[0][1] = -0.54f;	//	same
		a_matrix.entry[0][2] = 0.29f;	//	
		a_matrix.entry[1][0] = 0.51f;	//	
		a_matrix.entry[1][1] = 0.84f;	//	same
		a_matrix.entry[1][2] = 0.19f;	//	
		a_matrix.entry[2][0] = -0.34f;	//	const
		a_matrix.entry[2][1] = 0.0f;	//	const same
		a_matrix.entry[2][2] = 0.93f;	//	const
	}

//	vertical axe turn
//	[0.018283, 0.011742, -0.999764]
//	[-0.540305, 0.841469, 0.000002]
//	[0.841270, 0.540178, 0.021729]
//
//	lateral axe turn
//	[0.790722, 0.507721, -0.342020]
//	[-0.540305, 0.841469, 0.000002]
//	[0.287801, 0.184794, 0.939693]
}
void Leviathan::SetHitRotation(RE::NiPoint3& a_angles, const bool a_vertical)
{
	if  (a_vertical) {
	//	a_angles.x = 0.f;		//	const
		a_angles.y = -88.6f;	//	const
		a_angles.z = -32.7f;	//	
	} else {
	//	a_angles.x = 0.f;		//	const
		a_angles.y = -20.f;		//	const
		a_angles.z = -32.7f;	//	
	}
}
/*
void Leviathan::Charge(const int a_DurationSec, const float a_Magnitude)
{
	auto levi = WeaponIdentify::LeviathanAxe;
	auto ench = levi->formEnchanting->data.baseEnchantment;
	if (ench && ench->avEffectSetting) {
		spdlog::debug("levi's enchantment is: {}", ench->GetName());
		if (ench->avEffectSetting->HasArchetype(RE::EffectSetting::Archetype::kDualValueModifier)
		 || ench->avEffectSetting->HasArchetype(RE::EffectSetting::Archetype::kValueModifier)) {
			spdlog::debug("enchantment has required archetypes");
			if (ench->effects[0]) {
				static auto& mag = ench->effects[0]->effectItem.magnitude;
				spdlog::debug("mag reference getted");
				mag *= a_Magnitude;
				spdlog::debug("magnitude buffing from {} to: {}", mag / a_Magnitude, mag);
				axeChargeThread = std::jthread([=](){
					int sec = 0;
					while (sec >= a_DurationSec) {
						if (RE::UI::GetSingleton()->GameIsPaused()) continue;
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
						sec++;
					}
					mag /= a_Magnitude;
					ench->~EnchantmentItem();
					spdlog::debug("magnitude resetting from {} to: {}", mag * a_Magnitude, mag);
				}); 
				axeChargeThread.detach();
			} else spdlog::warn("WEIRD levi enchantment not has effect.");
		} else spdlog::debug("levi's enchantment is not expected archetype.");
	} else {
		spdlog::debug("levi not has any enchantment, levi is enchanting...");
		if (EnchCharge) {
			ench = EnchCharge;
			static auto& mag = ench->effects[0]->effectItem.magnitude;
			const float leviDam = static_cast<float>(levi->attackDamage);
			mag = a_Magnitude * leviDam / 2;
			spdlog::info("levi charge frost damage buff is: {}", mag);
			axeChargeThread = std::jthread([=](){
				int sec = 0;
				while (sec >= a_DurationSec) {
					if (RE::UI::GetSingleton()->GameIsPaused()) continue;
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					sec++;
				}
			//	ench->formFlags |= rFlag::kDeleted;
				ench->~EnchantmentItem();
				spdlog::debug("levi charge end");});
			axeChargeThread.detach();
		} else spdlog::warn("can't find frost enchantment");
	}
}
*/
//DRAUPNIR SPEAR
void Draupnir::Throw()
{
	const auto AnArchos = PlayerCharacter::GetSingleton();
//	auto& runtimeData = AnArchos->GetActorRuntimeData();
	if (WeaponIdentify::DraupnirSpear && SpellDraupnirProjL && AnArchos->GetEquippedObject(false) == WeaponIdentify::DraupnirSpear)
	{
		const auto draupnirDamage = static_cast<float>(WeaponIdentify::DraupnirSpear->attackDamage);
		float mag = draupnirDamage * WeaponIdentify::DamageMult;
		const auto effDraupnir = SpellDraupnirProjL->effects[0];
		auto& leviProjEffSetting = effDraupnir->effectItem;
		leviProjEffSetting.magnitude = mag;

		auto origin = WeaponIdentify::RHandBone->world.translate;
		RE::ProjectileHandle pHandle;
		RE::Projectile::ProjectileRot pRot = {AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
		RE::Projectile::LaunchData lData(AnArchos, origin, pRot, SpellDraupnirProjL);
		lData.weaponSource = WeaponIdentify::DraupnirSpear;
		if (lData.weaponSource->formEnchanting) lData.enchantItem = lData.weaponSource->formEnchanting;
		else spdlog::debug("draupnir not has a valid enchantment");

		RE::Projectile::Launch(&pHandle, lData);
	//	auto effBaseDraupnir = effDraupnir->baseEffect;
	//	if (effBaseDraupnir) {
	//		effBaseDraupnir->data.projectileBase->data.defaultWeaponSource = WeaponIdentify::LeviathanAxe;
	//		effBaseDraupnir->data.associatedForm = WeaponIdentify::LeviathanAxe;
	//	} else spdlog::warn("not found throwing effect!");
	//	DraupnirSpearProjBaseL->data.defaultWeaponSource = WeaponIdentify::DraupnirSpear;
	//	AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(SpellDraupnirProjL, false, nullptr, 1.f, false, mag, AnArchos);
	} else 	spdlog::info("Draupnir Spear is not equipped for throwing");
}
void Draupnir::Call(const float a_damage, const float a_force)	//detonate throwed draupnir spears like ragnarok
{
	int i = 0;	//	spear counter
	int j = 1;	//	explodable spear counter
	for (const auto proj : DraupnirSpearProjectiles) {
		i++;
		if (proj) {
			const auto& fFlags = proj->formFlags;
				spdlog::debug("{}. Draupnir flags: {:032b}", i, fFlags);
				/*	[crashed flags]
				00111001010010000101010001101000
				00111100110011110000010000000000
				10111111001001100001100000000000	// IMPORTANT CRASH
				00000000000000000000000000011110	// stucked to characters.
				00000000000000000000000000000 1 00	// stucked to characters. explosion timer working but not exploding. formID is null. kPlayable
				000000000000000000000010010 1 0000	// kNonOccluder. 4. from right
				000000000000000000000000000111 1 0	// kAltered. 1. from right
				01000011 1 00000000000000000000000	// kDestroyed. 23. from right
				00100011010011010110010111101000	// not has the kmoved flag but is stucked to a non actor object already
				11101011010000001011010110011000	// stucked non living object
				*/
				/*	[working flags]
				00000000000000000000000000001000	// kInitialized. (3. from right)
				00000000000000000000000000101000	// kDeleted (5. from right) is not problem
				00000000000000000000001000111000	// it says not has explosion
				*/
		//	if (fFlags & rFlag::kDestroyed) {fFlags &= ~rFlag::kDestroyed; spdlog::info("{}. Draupnir's destroyed flag removed", i);}
			if ((fFlags & rFlag::kInitialized)) {
				if (!(fFlags & rFlag::kDisabled)) {
					if (!(fFlags & rFlag::kStillLoading)) {
						auto& runtimeData = proj->GetProjectileRuntimeData();
						auto& pFlags = runtimeData.flags;
							spdlog::debug("{}. Draupnir runtime flags: {:032b}", i, pFlags);
							/*	[runtime flags]
							10000100011100011000000001000000 // IMPORTANT CRASH
							01111111011111111111111111111111 // crashed with all zero formFlags
							01000111000100001111101111110001 // not has kmoved flag while stucked to the ground

							10000100011100011000000101000000 //	stucked to objects.
							kGravityUpdateModel, kInited, kDestroyAfterHit, kAddedToManager, kAddedVisualEffectOnGround, kAutoAim, kProcessedImpacts

							10000110001101011000000001000000 //	stucked to characters.
							10000110001100011000000001000000 // stucked to characters. explosion time edit working but not exploding.
							10000110001100011000000001000000 // stucked to characters. explosion time edit working but not exploding. 3d is dissapeared.
							10100110000100011000000001000000

							10000110001100011000000011000000 // it says not has explosion

							10000100011100011000000101000100 //	to havok.	kUnk2 = kIsHavok
							10000110001100011000000001000100 // to havok, caused ctd

							10000100001000011000000101000000 //	still moving.
							kGravityUpdateModel, kInited, kDestroyAfterHit, kAddedToManager, kAutoAim,

							10000110001100011000000001000000 // i saw it with kNone form flags.

							10000100011100011000000001000000 // exploded, explosion timer < 0, formID = 0x0
							*/
						if  (!(pFlags & (1 << 8))) {
							if (!(pFlags & (1 << 2)) && !(pFlags & (1 << 22)) && (pFlags & (1 << 21)) && (pFlags & (1 << 25))) {	//	means hitted to a living target
								if (!SpellDraupnirsCallProjL) return;
#ifdef EXPERIMENTAL
							//	experimental
								const auto AnArchos = RE::PlayerCharacter::GetSingleton();
								for (auto hitBone : DraupnirSpearHitBones) {
									const auto target = DraupnirSpearHitActors[i - 1];
									if (hitBone && target) {
										const float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.3f, -0.1f);
										const auto delay = randomFloat + static_cast<float>(j) / 2.f;
										std::jthread delayedCast([=]() {
    										DelayedCast(AnArchos, SpellDraupnirsCallProjL, hitBone, delay);
											target->RemoveExtraArrows3D();
											DraupnirSpearHitBones[i - 1] = nullptr;
											DraupnirSpearHitActors[i - 1] = nullptr;
										});
										delayedCast.detach();
									}
								}
								j++;
#endif
							}	continue;
						}
						if (const auto expl = runtimeData.explosion) {
							if (pFlags & (1 << 15)) {	//	kDestroyAfterHit flag required, otherwise the explosion will cast every moment
								if (!(pFlags & 0u/*(1 << 31)*/)) {	// I don't know why but causing crashes without kMoved flag
									if (expl->GetFormID()) spdlog::debug("{}. draupnir explosion is: {:08x}", i, expl->formID);
									else spdlog::debug("{}. draupnir explosion not has formID", i);
								//	if (!(pFlags & (1 << 22))) pFlags |= (1 << 22);	//	i tried it for make working the explosions after hitting living targets, but not worked
								//	if (pFlags & (1 << 13)) pFlags &= ~(1 << 13);
								//	if (pFlags & (1 << 25)) pFlags &= ~(1 << 25);
								//		spdlog::debug("{}. Draupnir updated runtime flags: {:032b}", i, pFlags);
									expl->data.damage	= a_damage;
									expl->data.force	= a_force;

									if (!(pFlags & (1 << 2)) && !(pFlags & (1 << 22)) && (pFlags & (1 << 21)) && (pFlags & (1 << 25))) {	//	means hitted to a living target
										if (!SpellDraupnirsCallProjL) return;
#ifdef EXPERIMENTAL
									//	experimental
										const auto AnArchos = RE::PlayerCharacter::GetSingleton();
										for (auto hitBone : DraupnirSpearHitBones) {
											const auto target = DraupnirSpearHitActors[i - 1];
											if (hitBone && target) {
												const float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.1f);
												const auto delay = randomFloat + static_cast<float>(j) / 4.f;
												std::jthread delayedCast([=]() {
    												DelayedCast(AnArchos, SpellDraupnirsCallProjL, hitBone, delay);
													target->RemoveExtraArrows3D();
													DraupnirSpearHitBones[i - 1] = nullptr;
													DraupnirSpearHitActors[i - 1] = nullptr;
												});
												delayedCast.detach();
											}
										}
										j++;
#endif
										continue;
									}

									if (runtimeData.explosionTimer > 0.f) {
										if (!(pFlags & (1 << 2)) && !(pFlags & (1 << 20))) runtimeData.explosionTimer = 0.02f;	//	explodes immediately if is not stucked anywhere
										else {
										//	float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.1f);
											runtimeData.explosionTimer = static_cast<float>(j) / 4.f;// + randomFloat;
										} j++;	//	update explodable spear counter
									} else spdlog::info("{}. Draupnir not has explosion timer", i);

									if (const auto formID = proj->GetFormID()) spdlog::info("{}. Draupnir Spear's ({:08x}) explositon timer is: {}", i, formID, runtimeData.explosionTimer);
									else {spdlog::debug("{}. Draupnir Spear's (null) explositon timer is: {}", i, runtimeData.explosionTimer);}
								} else {spdlog::debug("{}. Draupnir is not exist", i);}
							} else {spdlog::debug("{}. Draupnir is not destructable (runtimeFlag)", i);}
						} else {spdlog::info("{}. Draupnir not has explosion", i);}
					} else {spdlog::debug("{}. Draupnir is kStillLoading (formFlag)", i);}
				} else {spdlog::debug("{}. Draupnir is kDisabled (formFlag)", i);}
			} else {spdlog::debug("{}. Draupnir is not inited (formFlag)", i);}
		} else {spdlog::debug("{}. Draupnir Spear is nullptr", i);}
	}
#ifdef EXPERIMENTAL
//	experimental
	if (!SpellDraupnirsCallProjL) return;
	const auto AnArchos = RE::PlayerCharacter::GetSingleton();
	int k = 0;
	int l = 1;
	for (auto hitBone : DraupnirSpearHitBones) {
		const auto proj = DraupnirSpearProjectiles[k];
		const auto target = DraupnirSpearHitActors[k];
		if (proj && hitBone && target) {
			const float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.1f);
			const auto delay = randomFloat + static_cast<float>(l) / 4.f;
			l++;
			std::jthread delayedCast([=]() {
    			DelayedCast(AnArchos, SpellDraupnirsCallProjL, hitBone, delay);
				target->RemoveExtraArrows3D();
			});
			delayedCast.detach();
		}
		k++;
	}
#endif
}

using EventChecker = RE::BSEventNotifyControl;
bool AnimationEventTracker::Register()
{
	const auto pc = PlayerCharacter::GetSingleton();

	bool bSinked = false;
	bool bSuccess = pc->AddAnimationGraphEventSink(AnimationEventTracker::GetSingleton());
	if (bSuccess) {
		spdlog::info("Registered {}", typeid(BSAnimationGraphEvent).name());
	} else {
		BSAnimationGraphManagerPtr graphManager;
		pc->GetAnimationGraphManager(graphManager);
		if (graphManager) {			
			for (auto& animationGraph : graphManager->graphs) {
				if (bSinked) {
					break;
				}
				auto eventSource = animationGraph->GetEventSource<BSAnimationGraphEvent>();
				for (auto& sink : eventSource->sinks) {
					if (sink == AnimationEventTracker::GetSingleton()) {
						bSinked = true;
						break;
					}
				}
			}
		}

		if (!bSinked) {
			spdlog::info("Failed to register {}", typeid(BSAnimationGraphEvent).name());
		}		
	}
	return bSuccess || bSinked;
}

EventChecker AnimationEventTracker::ProcessEvent(const BSAnimationGraphEvent* a_event, BSTEventSource<BSAnimationGraphEvent>* a_eventSource)
{
	if (a_event) {
		std::string eventTag = a_event->tag.data();
	//	spdlog::debug("Event: {}", eventTag);
/*		if (eventTag == Config::ThrowEvent) {
			if (Leviathan::GetThrowState() == Leviathan::ThrowState::kThrowable) {
				Leviathan::Throw(false);
			}
			else spdlog::warn("Levi is not throwable");
		}
		else if (eventTag == Config::ThrowEventV) {
			if (Leviathan::GetThrowState() == Leviathan::ThrowState::kThrowable) {
				Leviathan::Throw(true);
			}
			else spdlog::warn("Levi is not throwable");
		}
		else if (eventTag == Config::CallEvent) {
			if (Leviathan::throwState == Leviathan::ThrowState::kCanArrive
			 || Leviathan::throwState == Leviathan::ThrowState::kThrown)
				Leviathan::Arrive();
		}
		else if (eventTag == Config::CallAttackEvent) {
			if (WeaponIdentify::isLeviathanAxe) {
				const auto AnArchos = PlayerCharacter::GetSingleton();
				AnArchos->NotifyAnimationGraph("MCO_AttackInitiate");
			}
			else spdlog::warn("Levi is not callable");
		}
		else if (eventTag == Config::ThrowEndEvent) {
			if (WeaponIdentify::isLeviathanAxe) {
				const auto AnArchos = PlayerCharacter::GetSingleton();
				const auto eqManager = RE::ActorEquipManager::GetSingleton();
				if (AnArchos && eqManager)
					eqManager->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, true, false, false, true, nullptr);
				else spdlog::warn("WEIRD!! can't unequipped levi");
			}
		}
		else if (eventTag == Config::ChargeLeviEvent) {
	//		Leviathan::Charge(18, 1.8f);	causing crashes
		}

		else if (eventTag == Config::SpearThrowEvent) {
			Draupnir::Throw();
		}
		else if (eventTag == Config::DraupnirsCallEvent) {
			Draupnir::Call(1.f);
		}
*/
//	const auto AnArchos = PlayerCharacter::GetSingleton();
		switch (hash(eventTag.data(), eventTag.size())) {
		// Start phase
		case "CallWeapon"_h:
		//	if (Leviathan::throwState == Leviathan::ThrowState::kCanArrive
		//	 || Leviathan::throwState == Leviathan::ThrowState::kThrown)
				Leviathan::Arrive();
		//	else spdlog::warn("!!the ax did not come");
			break;
		case "CatchLevi"_h:
			WeaponIdentify::WeaponCheck();
			if (WeaponIdentify::LeviathanAxe) {
				const auto AnArchos = PlayerCharacter::GetSingleton();
				if (AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe) {
					Leviathan::Catch(Leviathan::LeviathanAxeProjectileA, AnArchos);
				}
			} else spdlog::warn("Levi is not catched!!");
			break;
		case "LeviCallAttack"_h:	//event: attackPowerStartInPlace, attackStart, PowerAttack [IDLE:000E8456], NormalAttack [IDLE:00013215]
			if (WeaponIdentify::LeviathanAxe) {
				const auto AnArchos = PlayerCharacter::GetSingleton();
				const auto eqManager = RE::ActorEquipManager::GetSingleton();
			//	auto& actorState1	= AnArchos->AsActorState()->actorState1;
			//	auto& actorState2	= AnArchos->AsActorState()->actorState2;
			//	if (!actorState1.swimming) {
					if (AnArchos && eqManager) {
						eqManager->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe, nullptr, 1U, nullptr, false, true, false, true);
			//			spdlog::debug("we tricked the game :))");
			//			actorState1.swimming = true;
			//			actorState2.weaponState = RE::WEAPON_STATE::kDrawn;
			//			actorState1.meleeAttackState = RE::ATTACK_STATE_ENUM::kNextAttack;
			//			actorState1.swimming = false;
					}
			//	} else {
			//		spdlog::debug("levi call attack triggered in water");
			//		if (AnArchos && eqManager)
			//			eqManager->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, true, false, false, true, nullptr);
			//	}
			}
			else spdlog::warn("Levi is not callable");
			break;
		case "ThrowWeapon"_h:
			if (Leviathan::GetThrowState() == Leviathan::ThrowState::kThrowable) {
				Leviathan::Throw(false);
			}
			else spdlog::warn("Levi is not throwable");
			break;
		case "ThrowWeaponV"_h:
			if (Leviathan::GetThrowState() == Leviathan::ThrowState::kThrowable) {
				Leviathan::Throw(true);
			}
			else spdlog::warn("Levi is not throwable");
			break;
		case "ThrowAttackEnd"_h:
			if (WeaponIdentify::isLeviathanAxe) {
				const auto AnArchos = PlayerCharacter::GetSingleton();
				const auto eqManager = RE::ActorEquipManager::GetSingleton();
				auto& actorState1	= AnArchos->AsActorState()->actorState1;
			//	auto& actorState2	= AnArchos->AsActorState()->actorState2;
				if (!actorState1.swimming) {
				//	spdlog::debug("we tricked the game :))");
					actorState1.swimming = true;
					if (AnArchos && eqManager) {
						eqManager->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe, nullptr, 1U, nullptr, true, false, true, true);
					} else spdlog::warn("WEIRD!! can't unequipped levi");
					actorState1.swimming = false;
				} else {
					spdlog::debug("levi throwed in water");
					if (AnArchos && eqManager)
						eqManager->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe, nullptr, 1U, nullptr, true, false, true, true);
					else spdlog::warn("WEIRD!! can't unequipped levi");
				}
			}
			break;
		case "LeviCharge"_h:
	//		Leviathan::Charge(18, 1.8f);	causing crashes
			break;
		case "ThrowSpear"_h:
			Draupnir::Throw();
			break;
		case "DraupnirsCall"_h:
			if (WeaponIdentify::DraupnirSpear) {
				const float draupnirDamage = WeaponIdentify::DraupnirSpear->attackDamage;
				const float damage = draupnirDamage * WeaponIdentify::DamageMult;
				Draupnir::Call(damage, draupnirDamage);
			} else Draupnir::Call(10.f, 7.f);
			break;
		case "weaponDraw"_h:
			WeaponIdentify::WeaponCheck();
			if (WeaponIdentify::LeviathanAxe) {
				const auto AnArchos = PlayerCharacter::GetSingleton();
				if (AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe) {
					if (WeaponIdentify::LeviathanAxe->HasWorldModel()) {spdlog::debug("Levi is throwable"); Leviathan::SetThrowState(Leviathan::ThrowState::kThrowable);}
					else spdlog::debug("Levi is not equipped for real");
				}
			}
			break;
	//	case "CastOKStart"_h:
	//	case "MCO_AttackInitiate"_h:
	//	case "MCO_PowerAttackInitiate"_h:
	//	case "TDM_AttackStart"_h:
	//	case "Collision_AttackStart"_h:		//	axe calling attack state
	//		if (!WeaponIdentify::isRelic && AnArchos->IsAttacking() && SpellCastTracker::EffectAimButton && AnArchos->AsMagicTarget()->HasMagicEffect(SpellCastTracker::EffectAimButton)) {
	//			RE::ActorEquipManager::GetSingleton()->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, false, false, false, true);
	//			AnArchos->NotifyAnimationGraph("MCO_AttackInitiate");
	//		}
	//		break;
		}
	}

	    return EventChecker::kContinue;
}
/*
//	check spell cast events
EventChecker InputEventTracker::ProcessEvent(RE::InputEvent* const *a_event, RE::BSTEventSource<RE::InputEvent*> *a_eventSource)
{
	if (a_event || RE::UI::GetSingleton()->GameIsPaused()) return EventChecker::kContinue;
	auto event = *a_event;
	if (event->HasIDCode() || event->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) return EventChecker::kContinue;

	auto keyCode = event->AsIDEvent()->GetIDCode();
	if (AimButton <= 0u || keyCode <= 0u) return EventChecker::kContinue;

	auto device = event->device.get();
	uint32_t aimButton = AimButton;
	switch (device) {
	case RE::INPUT_DEVICE::kKeyboard:
		break;
	case RE::INPUT_DEVICE::kGamepad:
		if (aimButton <= 266u) aimButton += 266u;
		break;
	case RE::INPUT_DEVICE::kMouse:
		if (aimButton <= 256u) aimButton += 256u;
		break;
	default:
		return EventChecker::kContinue;
	}

	if (keyCode == aimButton) {
		auto AnArchos = RE::PlayerCharacter::GetSingleton();
		if (APIs::tdm && APIs::tdm->GetDirectionalMovementState()) {
			APIs::result =	APIs::tdm->RequestDisableDirectionalMovement(SKSE::GetPluginHandle());
			if (APIs::result == TDM_API::APIResult::OK || APIs::result == TDM_API::APIResult::AlreadyGiven) spdlog::debug("Request TDM API ok");
			if (APIs::result != TDM_API::APIResult::OK && APIs::result != TDM_API::APIResult::AlreadyGiven) spdlog::debug("Request TDM API not ok");
			AnArchos->NotifyAnimationGraph("PickNewIdle");
			spdlog::info("aiming...");
				return EventChecker::kContinue;
		}
		if (APIs::tdm && !APIs::tdm->GetDirectionalMovementState()) {
			APIs::result =	APIs::tdm->ReleaseDisableDirectionalMovement(SKSE::GetPluginHandle());
			if (APIs::result == TDM_API::APIResult::OK || APIs::result == TDM_API::APIResult::AlreadyGiven) spdlog::debug("Release TDM API ok");
			if (APIs::result != TDM_API::APIResult::OK && APIs::result != TDM_API::APIResult::AlreadyGiven) spdlog::debug("Release TDM API not ok");
			AnArchos->NotifyAnimationGraph("PickNewIdle");
				return EventChecker::kContinue;
		}
	}	return EventChecker::kContinue;
}

void InputEventTracker::Register() {
            spdlog::info("input device manager registering...");
    auto sourceHolder = RE::BSInputDeviceManager::GetSingleton(); 
    if (sourceHolder) {
        sourceHolder->AddEventSink(InputEventTracker::GetSingleton());
            spdlog::info("input event sink registered!");
    } else  spdlog::warn("input event sink not registered!");
};
*/
/*
//	check spell cast events
EventChecker SpellCastTracker::ProcessEvent(const RE::TESSpellCastEvent* a_event, RE::BSTEventSource<RE::TESSpellCastEvent>* a_eventSource)
{
	if (a_event) {
		spdlog::debug("spell casted");
		auto sp = a_event->spell;
		if (sp) {
			if (sp == SpellAimButton->GetFormID()) {
				auto AnArchos = RE::PlayerCharacter::GetSingleton();
				if (EffectAimButton && AnArchos->AsMagicTarget()->HasMagicEffect(EffectAimButton)) {
					if (APIs::tdm && APIs::tdm->GetDirectionalMovementState()) {
						APIs::result =	APIs::tdm->RequestDisableDirectionalMovement(SKSE::GetPluginHandle());
						if (APIs::result == TDM_API::APIResult::OK || APIs::result == TDM_API::APIResult::AlreadyGiven) spdlog::debug("Request TDM API ok");
						if (APIs::result != TDM_API::APIResult::OK && APIs::result != TDM_API::APIResult::AlreadyGiven) spdlog::debug("Request TDM API not ok");
						AnArchos->NotifyAnimationGraph("PickNewIdle");
						spdlog::info("aiming...");
							return EventChecker::kContinue;
					}
				}
				if (EffectAimButton && !AnArchos->AsMagicTarget()->HasMagicEffect(EffectAimButton)) {
					if (APIs::tdm && !APIs::tdm->GetDirectionalMovementState()) {
						APIs::result =	APIs::tdm->ReleaseDisableDirectionalMovement(SKSE::GetPluginHandle());
						if (APIs::result == TDM_API::APIResult::OK || APIs::result == TDM_API::APIResult::AlreadyGiven) spdlog::debug("Release TDM API ok");
						if (APIs::result != TDM_API::APIResult::OK && APIs::result != TDM_API::APIResult::AlreadyGiven) spdlog::debug("Release TDM API not ok");
						AnArchos->NotifyAnimationGraph("PickNewIdle");
							return EventChecker::kContinue;
					}
				}
			}
		}
	}	return EventChecker::kContinue;
}

void SpellCastTracker::Register() {
    auto sourceHolder = RE::ScriptEventSourceHolder::GetSingleton(); 
    if (sourceHolder) {
        sourceHolder->AddEventSink(SpellCastTracker::GetSingleton());
            spdlog::info("Spell cast event sink registered!");
    } else  spdlog::warn("Spell cast event sink not registered!");
};
*/