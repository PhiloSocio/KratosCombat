#include "util.h"
#include "MainKratosCombat.h"
//#include "API/TrueDirectionalMovementAPI.h"

using namespace Util;
//static std::jthread axeEquipThread;
//static std::jthread axeChargeThread;
static void DelayedCast(RE::Actor* a_caster, RE::SpellItem* a_spell, RE::TESObjectREFR* a_target, float a_delaySec)
{
	if (!a_spell) return;
	if (!a_caster) return;
	int delay = static_cast<int>(a_delaySec * 1000.f);
	spdlog::debug("draupnir's call start...");
	std::this_thread::sleep_for(std::chrono::milliseconds(delay));
	if (!a_spell) return;
	if (!a_caster) return;
	spdlog::debug("draupnir's call triggered!");
	a_caster->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(a_spell, false, a_target, 1.f, false, 10.f, a_caster);
	spdlog::debug("draupnir's call end!");
}

//WEAPON IDENTIFICATION
void WeaponIdentify::WeaponIdentifier(TESObjectWEAP* a_RHandWeapon)
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

	auto AnArchos = PlayerCharacter::GetSingleton();
	if		(AnArchos->GetNodeByName("Weapon"))					{RHandBone = AnArchos->GetNodeByName("Weapon");}
	else if	(AnArchos->GetNodeByName("NPC R MagicNode [RMag]"))	{RHandBone = AnArchos->GetNodeByName("NPC R MagicNode [RMag]");}
	else if	(AnArchos->GetNodeByName("NPC R Finger20 [RF20]"))	{RHandBone = AnArchos->GetNodeByName("NPC R Finger20 [RF20]");}
	else if	(AnArchos->GetNodeByName("NPC R Hand [RHnd]"))		{RHandBone = AnArchos->GetNodeByName("NPC R Hand [RHnd]");}
		spdlog::debug("Right hand bone is {}", RHandBone->name);
	if (AnArchos->GetEquippedObject(false)){
		if (AnArchos->GetEquippedObject(false)->IsWeapon()){
			auto RHandWeapon = AnArchos->GetEquippedObject(false)->As<TESObjectWEAP>();
				return WeaponIdentifier(RHandWeapon);
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

static void ReLaunch(RE::Projectile* a_this, RE::NiPoint3* a_dV)
{
	using func_t = decltype(&ReLaunch);
	REL::Relocation<func_t> func{ RELOCATION_ID(43007, 0) };
	return func(a_this, a_dV);
}
inline void Leviathan::SetStartPos(RE::NiPoint3& a_point)
{
	auto stuckedLevi =	LastLeviProjectile ? LastLeviProjectile : nullptr;
	if (!stuckedLevi) stuckedLevi =	(LeviathanAxeProjectileL ? LeviathanAxeProjectileL : 
									(LeviathanAxeProjectileH ? LeviathanAxeProjectileH : nullptr));
	if (stuckedLevi)	a_point = stuckedLevi->data.location;
	else spdlog::debug("we cant get leviathan's stucked proj");

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
}
void Leviathan::Throw(bool isVertical)
{
	auto AnArchos = PlayerCharacter::GetSingleton();
	auto leviThrowSpell = (isVertical ? SpellLeviProjH : SpellLeviProjL);
	if (WeaponIdentify::LeviathanAxe && leviThrowSpell && AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe) 
	{	//switching to projectile
		auto leviDamage = static_cast<float>(WeaponIdentify::LeviathanAxe->attackDamage);
		auto mag = leviDamage + (isVertical ? leviDamage / 2.f : 0.f);
	//	auto leviProjBaseEff = leviThrowSpell->effects[0]->baseEffect;
	//	if (leviProjBaseEff) {
	//		leviProjBaseEff->data.projectileBase->data.defaultWeaponSource = WeaponIdentify::LeviathanAxe;
	//		leviProjBaseEff->data.associatedForm = WeaponIdentify::LeviathanAxe;
	//	} else spdlog::debug("not found throwing effect!");
//
//		auto extraData = AnArchos->extraList;
//		extraData.SetInventoryChanges(RE::InventoryChanges)
	//	RE::ActorEquipManager::GetSingleton()->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, false, false, false, true, nullptr);
	//	AnArchos->RemoveWeapon(RE::BIPED_OBJECT::kOneHandAxe);
	//	AnArchos->NotifyAnimationGraph("UnequipNoAnim");
//

		AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(leviThrowSpell, false, nullptr, 1.f, false, mag, AnArchos);
		if (!AnArchos->HasSpell(Config::SpellBHstate)) AnArchos->AddSpell(Config::SpellBHstate);
		isAxeCalled = false;
		isAxeThrowed = true;
		SetThrowState(ThrowState::kThrown);

		if (Leviathan::leviStuckedBone)		Leviathan::leviStuckedBone 	= nullptr;
		if (Leviathan::leviStuckedActor)	Leviathan::leviStuckedActor	= nullptr;
			spdlog::info("Leviathan Axe throwed, damage is: {}", mag);
		if (AnArchos->HasSpell(SpellCatchLevi)) AnArchos->RemoveSpell(SpellCatchLevi);
	} else spdlog::info("Leviathan Axe is not equipped for throwing");
}
void Leviathan::Arrive()
{
	spdlog::debug("Levi is calling...");
	auto stuckedLevi =	LastLeviProjectile ? LastLeviProjectile : nullptr;
	if (!stuckedLevi)	stuckedLevi = (LeviathanAxeProjectileL ? LeviathanAxeProjectileL : (LeviathanAxeProjectileH ? LeviathanAxeProjectileH : nullptr));
	auto AnArchos = PlayerCharacter::GetSingleton();

	if (SpellLeviProjA) {
		isAxeCalled = true;
		isAxeThrowed = false;

		if (stuckedLevi) leviPosition = stuckedLevi->data.location;
	//	float leviDamage = 1.f;
	//	if (WeaponIdentify::LeviathanAxe) leviDamage = static_cast<float>(WeaponIdentify::LeviathanAxe->attackDamage);
	//	AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(SpellLeviProjA, false, nullptr, 1.f, false, leviDamage, AnArchos);

		RE::NiPoint3 startPoint = leviPosition;
		SetStartPos(startPoint);
		RE::ProjectileHandle pHandle;
		RE::Projectile::ProjectileRot pRot = {AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
	//	RE::Projectile::LaunchData lData;
	//	{
	//	lData.shooter = AnArchos;
	//	lData.origin = startPoint;
	//	lData.spell = SpellLeviProjA;
	//	lData.projectileBase = LeviProjBaseA;
	//	lData.angleX = AnArchos->GetAimAngle();
	//	lData.angleZ = AnArchos->GetAimHeading();
	//	lData.contactNormal = {0.0f, 0.0f, 0.0f};
	//	lData.combatController = AnArchos->GetActorRuntimeData().combatController;
	//	lData.weaponSource = WeaponIdentify::LeviathanAxe;
	//	lData.parentCell = AnArchos->GetParentCell();
	//	lData.castingSource = RE::MagicSystem::CastingSource::kRightHand;
	//	lData.enchantItem = WeaponIdentify::LeviathanAxe->formEnchanting->data.baseEnchantment;
	//	lData.noDamageOutsideCombat = true;
	//	lData.ammoSource = nullptr;
	//	lData.unk50 = nullptr;
	//	lData.desiredTarget = nullptr;
	//	lData.unk60 = 0.0f;
	//	lData.unk64 = 0.0f;
	//	lData.poison = nullptr;
	//	lData.area = 0;
	//	lData.power = 1.0f;
	//	lData.scale = 1.0f;
	//	lData.alwaysHit = false;
	//	lData.autoAim = false;
	//	lData.useOrigin = true;
	//	lData.deferInitialization = false;
	//	lData.forceConeOfFire = false;
	//	}
		if (WeaponIdentify::LeviathanAxe) {
		//	RE::Projectile::Launch(&pHandle, lData);
			RE::Projectile::LaunchSpell(&pHandle, AnArchos, SpellLeviProjA, startPoint, pRot);
			SetThrowState(Leviathan::ThrowState::kArriving);
		} else spdlog::warn("you don't have the axe for calling!");
		if (Config::SpellBHstate && AnArchos->HasSpell(Config::SpellBHstate)) AnArchos->RemoveSpell(Config::SpellBHstate);

/*	*/	if (stuckedLevi) {
			auto& fFlags = stuckedLevi->formFlags;
			auto& runtimeData = stuckedLevi->GetProjectileRuntimeData();
			auto& pFlags = runtimeData.flags;
			if (!(pFlags & (1 << 25))) pFlags |= (1 << 25);
			spdlog::debug("levi destroyed before call");

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
	if (a_levi) {
		auto& runtimeData = a_levi->GetProjectileRuntimeData();
		runtimeData.flags |= (1 << 25);								//	set as destroyed, RE::Projectile::Flags::kDestroyed
	}

	if (a_justDestroy || !a_player) return;

	if (EffCatchLevi && SpellCatchLevi && !a_player->AsMagicTarget()->HasMagicEffect(EffCatchLevi)) {
		a_player->AddSpell(SpellCatchLevi);
	} else spdlog::debug("WEIRD!! you already in catch spell effect");

	if (throwState == ThrowState::kCanArrive) SetThrowState(ThrowState::kArrived);
	isAxeCalled = false;

	if (WeaponIdentify::LeviathanAxe)
		RE::ActorEquipManager::GetSingleton()->EquipObject(a_player, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, false, false, true, true);
	else spdlog::warn("you not have the leviathan axe");
	spdlog::debug("Levi proj catched");
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
	auto AnArchos = PlayerCharacter::GetSingleton();
	auto& runtimeData = AnArchos->GetActorRuntimeData();
	if (WeaponIdentify::DraupnirSpear && SpellDraupnirProjL && AnArchos->GetEquippedObject(false) == WeaponIdentify::DraupnirSpear)
	{
		auto draupnirDamage = static_cast<float>(WeaponIdentify::DraupnirSpear->attackDamage);
		auto mag = draupnirDamage;
	//	if (runtimeData.currentProcess && runtimeData.currentProcess->high) {
	//		auto atkData = runtimeData.currentProcess->high->attackData.get();
	//		if (atkData) {
	//			atkData->data.flags.any(RE::AttackData::AttackFlag::kPowerAttack);
	//			mag *= 1.2;
	//		}
	//	}
		auto effDraupnir = SpellDraupnirProjL->effects[0]->baseEffect;
		DraupnirSpearProjBaseL->data.defaultWeaponSource = WeaponIdentify::DraupnirSpear;
		effDraupnir->data.associatedForm = WeaponIdentify::DraupnirSpear;
		AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(SpellDraupnirProjL, false, nullptr, 1.f, false, mag, AnArchos);
	} else 	spdlog::info("Draupnir Spear is not equipped for throwing");
}
void Draupnir::Call(const float a_magnitude)	//detonate throwed draupnir spears like ragnarok
{
	int i = 0;	//	spear counter
	int j = 1;	//	explodable spear counter
	for (auto proj : DraupnirSpearProjectiles) {
		i++;
		if (proj) {
			auto& fFlags = proj->formFlags;
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

							10000100011100011000000101000000 //	stucked to objects.
							kGravityUpdateModel, kInited, kDestroyAfterHit, kAddedToManager, kAddedVisualEffectOnGround, kAutoAim, kProcessedImpacts

							10000110001101011000000001000000 //	stucked to characters.
							10000110001100011000000001000000 // stucked to characters. explosion time edit working but not exploding.
							10000110001100011000000001000000 // stucked to characters. explosion time edit working but not exploding. 3d is dissapeared.

							10000110001100011000000011000000 // it says not has explosion

							10000100011100011000000101000100 //	to havok.	kUnk2 = kIsHavok
							10000110001100011000000001000100 // to havok, caused ctd

							10000100001000011000000101000000 //	still moving.
							kGravityUpdateModel, kInited, kDestroyAfterHit, kAddedToManager, kAutoAim,

							10000110001100011000000001000000 // i saw it with kNone form flags.

							10000100011100011000000001000000 // exploded, explosion timer < 0, formID = 0x0
							*/
						auto expl = runtimeData.explosion;
						if (expl) {
							if (pFlags & (1 << 15)) {	//	kDestroyAfterHit flag required, otherwise the explosion will cast every moment
								if ((pFlags & (1 << 31))) {
									if (expl->GetFormID()) spdlog::debug("{}. draupnir explosion is: {:08x}", i, expl->formID);
									else spdlog::debug("{}. draupnir explosion not has formID", i);
								//	if (!(pFlags & (1 << 22))) pFlags |= (1 << 22);	//	i tried it for make working the explosions after hitting living targets, but not worked
								//	if (pFlags & (1 << 13)) pFlags &= ~(1 << 13);
								//	if (pFlags & (1 << 25)) pFlags &= ~(1 << 25);
								//		spdlog::debug("{}. Draupnir updated runtime flags: {:032b}", i, pFlags);
									expl->data.damage	= a_magnitude;
									expl->data.force	= a_magnitude;

								//	experimental
									if (!(pFlags & (1 << 2)) && !(pFlags & (1 << 22)) && (pFlags & (1 << 25))) {	//	means hitted to a living target
										j++;
										continue;
									}

									if (runtimeData.explosionTimer > 0.f) {
										if (!(pFlags & (1 << 2)) && !(pFlags & (1 << 20))) runtimeData.explosionTimer = 0.02f;	//	explodes immediately if is not stucked anywhere
										else {
											float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.2f);
											runtimeData.explosionTimer = randomFloat + static_cast<float>(j) / 2.f;
										} j++;	//	update explodable spear counter
									} else spdlog::info("{}. Draupnir not has explosion timer", i);

									auto formID = proj->GetFormID();
									if (formID) spdlog::info("{}. Draupnir Spear's ({:08x}) explositon timer is: {}", i, formID, runtimeData.explosionTimer);
									else {spdlog::debug("{}. Draupnir Spear's (null) explositon timer is: {}", i, runtimeData.explosionTimer);}
								} else {spdlog::debug("{}. Draupnir is not moved (runtimeFlag)", i);}
							} else {spdlog::debug("{}. Draupnir is not destructable (runtimeFlag)", i);}
						} else {spdlog::info("{}. Draupnir not has explosion", i);}
					} else {spdlog::debug("{}. Draupnir is kStillLoading (formFlag)", i);}
				} else {spdlog::debug("{}. Draupnir is kDisabled (formFlag)", i);}
			} else {spdlog::debug("{}. Draupnir is not inited (formFlag)", i);}
		} else {spdlog::debug("{}. Draupnir Spear is nullptr", i);}
	}
//	experimental	
//	int k = 0;
//	int l = 1;
//	for (auto hitBone : DraupnirSpearHitBones) {
//		auto proj = DraupnirSpearProjectiles[k];
//		auto target = DraupnirSpearHitActors[k];
//		if (proj && hitBone && target) {
//			if (SpellDraupnirsCallProjL) {
//				auto AnArchos = RE::PlayerCharacter::GetSingleton();
//				float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.2f);
//				auto delay = randomFloat + static_cast<float>(l) / 2.f;
//				l++;
//				std::jthread delayedCast([=]() {
//    				DelayedCast(AnArchos, SpellDraupnirsCallProjL, target, delay);
//					target->RemoveExtraArrows3D();
//				});
//				delayedCast.detach();
//			}
//		}
//		k++;
//	}
}

using EventChecker = RE::BSEventNotifyControl;
void AnimationEventTracker::Register()
{
	auto playerCharacter = PlayerCharacter::GetSingleton();
	bool bSuccess = playerCharacter->AddAnimationGraphEventSink(AnimationEventTracker::GetSingleton());
	if (bSuccess) {
		spdlog::info("Registered {}", typeid(BSAnimationGraphEvent).name());
	} else {
		BSAnimationGraphManagerPtr graphManager;
		playerCharacter->GetAnimationGraphManager(graphManager);
		bool bSinked = false;
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
				auto AnArchos = PlayerCharacter::GetSingleton();
				AnArchos->NotifyAnimationGraph("MCO_AttackInitiate");
			}
			else spdlog::warn("Levi is not callable");
		}
		else if (eventTag == Config::ThrowEndEvent) {
			if (WeaponIdentify::isLeviathanAxe) {
				auto AnArchos = PlayerCharacter::GetSingleton();
				auto eqManager = RE::ActorEquipManager::GetSingleton();
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
	auto AnArchos = PlayerCharacter::GetSingleton();
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
			if (WeaponIdentify::LeviathanAxe && AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe)
				Leviathan::Catch(Leviathan::LeviathanAxeProjectileA, AnArchos);
			else spdlog::warn("Levi is not catched!!");
			break;
		case "LeviCallAttack"_h:
			if (WeaponIdentify::LeviathanAxe) {
				auto AnArchos = PlayerCharacter::GetSingleton();
				auto eqManager = RE::ActorEquipManager::GetSingleton();
				auto& actorState1	= AnArchos->AsActorState()->actorState1;
				auto& actorState2	= AnArchos->AsActorState()->actorState2;
				if (!actorState1.swimming) {
					if (AnArchos && eqManager) {
					//	spdlog::debug("we tricked the game :))");
						actorState1.swimming = true;
						eqManager->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, true, false, false, true, nullptr);
					//	actorState2.weaponState = RE::WEAPON_STATE::kDrawn;
					//	actorState1.meleeAttackState = RE::ATTACK_STATE_ENUM::kNextAttack;
						actorState1.swimming = false;
					}
				} else {
					spdlog::debug("levi call attack triggered in water");
					if (AnArchos && eqManager)
						eqManager->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, true, false, false, true, nullptr);
				}
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
				auto AnArchos = PlayerCharacter::GetSingleton();
				auto eqManager = RE::ActorEquipManager::GetSingleton();
				auto& actorState1	= AnArchos->AsActorState()->actorState1;
			//	auto& actorState2	= AnArchos->AsActorState()->actorState2;
				if (!actorState1.swimming) {
					spdlog::debug("we tricked the game :))");
					actorState1.swimming = true;
					if (AnArchos && eqManager) {
						eqManager->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, true, false, false, true, nullptr);
					} else spdlog::warn("WEIRD!! can't unequipped levi");
					actorState1.swimming = false;
				} else {
					spdlog::debug("levi throwed in water");
					if (AnArchos && eqManager)
						eqManager->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, true, false, false, true, nullptr);
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
			Draupnir::Call(WeaponIdentify::DraupnirSpear ? WeaponIdentify::DraupnirSpear->attackDamage : 7.f);
			break;
		case "weaponDraw"_h:
			WeaponIdentify::WeaponCheck();
			if (WeaponIdentify::LeviathanAxe && AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe) {
				if (WeaponIdentify::LeviathanAxe->HasWorldModel()) {spdlog::debug("Levi is throwable"); Leviathan::SetThrowState(Leviathan::ThrowState::kThrowable);}
				else spdlog::debug("Levi is not equipped for real");

				if (Leviathan::SpellCatchLevi && AnArchos->AsMagicTarget()->HasMagicEffect(Leviathan::EffCatchLevi)) {AnArchos->RemoveSpell(Leviathan::SpellCatchLevi);}

				if (Leviathan::LeviathanAxeProjectileA
				 && (Leviathan::throwState == Leviathan::ThrowState::kArriving
				  || Leviathan::throwState == Leviathan::ThrowState::kArrived)) Leviathan::Catch(Leviathan::LeviathanAxeProjectileA, AnArchos, true);
				else spdlog::debug("levi not arrived or arriving!!");
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