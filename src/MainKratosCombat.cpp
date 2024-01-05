#include "util.h"
#include "MainKratosCombat.h"
#include "API/TrueDirectionalMovementAPI.h"

using namespace Util;
static std::jthread axeChargeThread;
//WEAPON IDENTIFICATION
void WeaponIdentify::WeaponIdentifier(TESObjectWEAP* a_RHandWeapon)
{
    auto RHandWeaponKWDlist = a_RHandWeapon->As<BGSKeywordForm>();
    auto RelicName = "not a Relic";

    if (RHandWeaponKWDlist->HasKeywordString(Config::LeviathanAxeKeyword)) {
    	isLeviathanAxe = true;
    	RelicName = "the Leviathan Axe";
		LeviathanAxe = a_RHandWeapon;
    	spdlog::info("{} is {}", LeviathanAxe->GetName(), RelicName);
			return;
	}
    if (RHandWeaponKWDlist->HasKeywordString(Config::BladeOfChaosKeyword)) {
    	isBladeOfChaos = true;
    	RelicName = "the Blade of Chaos";
		BladeOfChaos = a_RHandWeapon;
//			return;
	}
    if (RHandWeaponKWDlist->HasKeywordString(Config::DraupnirSpearKeyword)) {
    	isDraupnirSpear = true;
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
	spdlog::info("AnArchos is Barehanded");
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
void Leviathan::Throw(bool isVertical)
{
	auto AnArchos = PlayerCharacter::GetSingleton();
	auto leviThrowSpell = isVertical ? SpellLeviProjH : SpellLeviProjL;
	if (WeaponIdentify::LeviathanAxe && leviThrowSpell && AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe) 
	{	//switching to projectile
		auto leviDamage = static_cast<float>(WeaponIdentify::LeviathanAxe->attackDamage);
		auto mag = leviDamage + isVertical ? leviDamage / 2.f : 0.f;
		RE::ActorEquipManager::GetSingleton()->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe, nullptr, 1U, nullptr, false, false, false, true, nullptr);
		AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(leviThrowSpell, false, nullptr, 1.f, false, mag, AnArchos);
		if (!AnArchos->HasSpell(Config::SpellBHstate)) AnArchos->AddSpell(Config::SpellBHstate);
		isAxeCalled = false;
		isAxeThrowed = true;
		SetThrowState(ThrowState::kThrown);

		Leviathan::ThrowedLevi1 = nullptr;
		Leviathan::ThrowedLevi2 = nullptr;
			spdlog::info("Leviathan Axe throwed, damage is: {}", mag);
	//	if (AnArchos->HasSpell(SpellCatchLevi)) AnArchos->RemoveSpell(SpellCatchLevi);
	} else spdlog::info("Leviathan Axe is not equipped for throwing");
}
void Leviathan::Arrive()
{
	auto stuckedLevi = LeviathanAxeProjectileL ? LeviathanAxeProjectileL : LeviathanAxeProjectileH;
	auto AnArchos = PlayerCharacter::GetSingleton();

//	if (!ThrowedLevi1) {
//		spdlog::debug("WEIRD Leviathan Axe Projectile is null");
//		return;
//	}
//	if (!isAxeStucked) {
//		if (stuckedLevi) {
//			auto& runtimeData = stuckedLevi->GetProjectileRuntimeData();
//			auto pFlags = runtimeData.flags;
//			if (!(pFlags & (1 << 2)) && !(pFlags & (1 << 20))) return;
//			else spdlog::debug("Leviathan Axe is stucked to an actor");
//		} else return;
//	}

	if (SpellLeviProjA) {
		isAxeCalled = true;
		isAxeThrowed = false;
		AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(SpellLeviProjA, false, nullptr, 1.f, false, 1.f, AnArchos);
		SetThrowState(ThrowState::kArriving);
		if (AnArchos->HasSpell(Config::SpellBHstate)) AnArchos->RemoveSpell(Config::SpellBHstate);

		if (stuckedLevi) {
			auto& fFlags = stuckedLevi->formFlags;
			if ((fFlags & rFlag::kInitialized)) {
				if (!(fFlags & rFlag::kDisabled)) {
					if (!(fFlags & rFlag::kStillLoading)) {
						auto& runtimeData = stuckedLevi->GetProjectileRuntimeData();
						auto& pFlags = runtimeData.flags;
						auto expl = runtimeData.explosion;
						if (expl) {
							if (pFlags & (1 << 15)) {	//	kDestroyAfterHit flag required, otherwise the explosion will cast every moment
								if ((pFlags & (1 << 31))) {
									if (pFlags & (1 << 13)) pFlags &= ~(1 << 13);
									if (pFlags & (1 << 25)) pFlags &= ~(1 << 25);
									if (runtimeData.explosionTimer > 0.f) runtimeData.explosionTimer = 0.02f;
										spdlog::debug("Levi is detached!!");
								} else {spdlog::debug("Levi is not moved (runtimeFlag)");}
							} else {spdlog::debug("Levi is not destructable (runtimeFlag)");}
						} else {spdlog::info("Levi not has explosion");}
					} else {spdlog::debug("Levi is kStillLoading (formFlag)");}
				} else {spdlog::debug("Levi is kDisabled (formFlag)");}
			} else {spdlog::debug("Levi is not inited (formFlag)");}
		} else {spdlog::debug("Stucked Levi is nullptr!");}
	} else {spdlog::warn("WEIRD SpellLeviProjA is nullptr!");}
	spdlog::info("Levi is calling...");
	return;

	auto targetPoint = WeaponIdentify::RHandBone;
	NiPoint3 handPos = targetPoint->world.translate;

//	NiPoint3& leviPos = ThrowedLevi1->world.translate;
	NiPoint3& leviPos = stuckedLevi->data.location;

	float distance = handPos.GetDistance(leviPos);
	while (distance > Config::CatchingTreshold) {
		spdlog::debug("distance of stucked levi is: {}", distance);
	}

	if (WeaponIdentify::LeviathanAxe)
		RE::ActorEquipManager::GetSingleton()->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe, nullptr, 1U, nullptr, false, false, false, true);
	return;

	float deltaTime = 1 / 60.f;
	float speed = distance / Config::ArrivalTime;
		spdlog::debug("speed of stucked levi is: {}", speed);
	if (speed < Config::MinArrivalSpeed) {
		speed = Config::MinArrivalSpeed;
	}
	if (speed > Config::MaxArrivalSpeed) {
		speed = Config::MaxArrivalSpeed;
	}
/*//for cycle
	int step = 144;
	for (int i = 0; i <= step; i++) {
	
		targetPoint = AnArchos->GetNodeByName("NPC R Hand [RHnd]");

		if (targetPoint) {
			handPos = targetPoint->world.translate;

			distance = handPos.GetDistance(leviPos);
			
			float offset = (sqrtf(distance) * Config::ArrivalRoadCurveMagnitude);

			RE::NiPoint3 direction = (handPos - leviPos);
			direction.Unitize();
			
			RE::NiPoint3 crossDir = direction.UnitCross({0.f, 0.f, -1.f});
			RE::NiPoint3 midPoint = (handPos + leviPos) / 2;
			RE::NiPoint3 curvePoint = midPoint + crossDir * offset;

			RE::NiPoint3 curvyDir = (curvePoint - leviPos);
			curvyDir.Unitize();
	
			auto velocity = curvyDir * speed;

			leviPos += velocity * deltaTime;

			spdlog::debug("Levi is arriving {}...", distance);

			if (distance < Config::CatchingTreshold) {
				break;
			}
		}
		else {
			spdlog::debug("TargetPoint is nullptr");
			break;
		}
	}
	return;
*/
//	while cycle
	while (distance > Config::CatchingTreshold) {

		targetPoint = AnArchos->GetNodeByName("NPC R Hand [RHnd]");

		if (targetPoint) {
			handPos = targetPoint->world.translate;

			distance = handPos.GetDistance(leviPos);

			float offset = (sqrtf(distance) * Config::ArrivalRoadCurveMagnitude);

			RE::NiPoint3 direction	= (handPos - leviPos);
			direction.Unitize();

			RE::NiPoint3 crossDir 	= direction.UnitCross({0.f, 0.f, -1.f});
			RE::NiPoint3 midPoint 	= (handPos + leviPos) / 2;
			RE::NiPoint3 curvePoint	= midPoint + crossDir * offset;

			RE::NiPoint3 curvyDir = (curvePoint - leviPos);
			curvyDir.Unitize();

			RE::NiPoint3 velocity = curvyDir * speed * deltaTime;

			leviPos += velocity;

			spdlog::debug("Levi is arriving {}...", distance);
		} else {
			spdlog::debug("TargetPoint is nullptr");
			break;
		}
	} spdlog::debug("stucked Leviathan Axe arrived");
	return;
}
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

//DRAUPNIR SPEAR
void Draupnir::Throw()
{
	auto AnArchos = PlayerCharacter::GetSingleton();
	if (WeaponIdentify::DraupnirSpear && SpellDraupnirProjL && AnArchos->GetEquippedObject(false) == WeaponIdentify::DraupnirSpear)
	{
		auto draupnirDamage = static_cast<float>(WeaponIdentify::DraupnirSpear->attackDamage);
		auto mag = draupnirDamage;
		AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(SpellDraupnirProjL, false, nullptr, 1.f, false, mag, AnArchos);
	} else 	spdlog::info("Draupnir Spear is not equipped for throwing");
}
void Draupnir::Call(const float a_magnitude)	//detonate throwed draupnir spears like ragnarok
{
	int i = 0;	//	spear counter
	int j = 1;	//	explodable spear counter
	for (auto proj : DraupnirSpearProjectiles)
	{
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
									if (pFlags & (1 << 13)) pFlags &= ~(1 << 13);
									if (pFlags & (1 << 25)) pFlags &= ~(1 << 25);
										spdlog::debug("{}. Draupnir updated runtime flags: {:032b}", i, pFlags);
									expl->data.damage *= a_magnitude;
									expl->data.force *= a_magnitude;

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
	}/* return;

	for (int i = 0; i <= Config::DraupnirSpearCount; i++)
	{
		if (DraupnirSpearProjectiles[i]) {
		//	if (DraupnirSpearProjectiles[i]->IsDeleted()
		//	 || DraupnirSpearProjectiles[i]->IsDisabled()
		//	 || DraupnirSpearProjectiles[i]->IsIgnored()) {spdlog::info("{}. Draupnir is not accessible", i);}
			auto& fFlags = DraupnirSpearProjectiles[i]->formFlags;
				spdlog::debug("{}. Draupnir flags: {:032b}", i, fFlags);	//	[crash] 0. Draupnir flags: 00111001010010000101010001101000 / 00111100110011110000010000000000
			if (fFlags & rFlag::kDestroyed || fFlags & rFlag::kEmpty) {
				auto& runtimeData = DraupnirSpearProjectiles[i]->GetProjectileRuntimeData();
				auto expl = runtimeData.explosion;
				if (expl) {
					if (!(runtimeData.flags & static_cast<uint32_t>(pFlag::kDestroyed))) {
						expl->data.damage *= a_magnitude;
						expl->data.force *= a_magnitude;

						if (runtimeData.explosionTimer > 0) runtimeData.explosionTimer = static_cast<float>(i) / 2 + MathUtil::Algebra::generateRandomFloat(0.3f, 0.6f);
						else spdlog::info("{}. Draupnir not has explosion timer", i);

						auto formID = DraupnirSpearProjectiles[i]->GetFormID();
						if (formID) spdlog::info("{}. Draupnir Spear's ({:08x}) explositon timer is: {}", i, formID, runtimeData.explosionTimer);
						else 		spdlog::info("{}. Draupnir Spear's (null) explositon timer is: {}", i, runtimeData.explosionTimer);
					}	else 	   {spdlog::info("{}. Draupnir is destroyed (runtimeFlag)", i);}
				}
				else {spdlog::info("{}. Draupnir not has explosion", i);}
			}	else {spdlog::info("{}. Draupnir is destroyed (formFlag)", i);}
		}
		else spdlog::debug("{}. Draupnir Spear is nullptr", i);
	}
*/
}

/*
void CallLeviathan()
{
	RE::ArrowProjectile* Levi = nullptr;
//	RE::Projectile* Levi = nullptr;
	auto _ArrivalTime = Config::ArrivalTime;
	auto NearDistance = Config::CatchingTreshold;
	auto _Levi = WeaponIdentify::LeviathanAxe;
	if (!(Config::LeviathanAxeProjectileL) || !(Config::LeviathanAxeProjectileH)){
			spdlog::debug("Can't found projectile levi");
			return;
	};
	auto _ProjLeviL = Config::LeviathanAxeProjectileL;
	auto _ProjLeviH = Config::LeviathanAxeProjectileH;
		spdlog::debug("Calling.. {}", _ProjLeviL->GetName());
	if (!_ProjLeviH->Is3DLoaded() && !_ProjLeviH->Is3DLoaded()){
		spdlog::debug("There is no 3D loaded throwed Leviathan");
		return;
	}
	if (_ProjLeviL->Is3DLoaded()){
		spdlog::debug("{} is 3DLoaded", _ProjLeviL->GetName());
		Levi = _ProjLeviL;
	}
	if (_ProjLeviH->Is3DLoaded()){
		Levi = _ProjLeviH;
	}
		spdlog::debug("Calling...");

	bool isHit = Levi->GetSpeed() < 2000;
//	auto LeviPos = Levi->GetPosition();
//	auto LeviLoc = Levi->GetCurrentLocation();
	auto AnArchos = PlayerCharacter::GetSingleton();
	auto AnArchosRef = AnArchos->AsReference();
	auto AnArchosHand = AnArchosRef->GetNodeByName("NPC R Hand [RHnd]");
		spdlog::debug("Calling....");
//	auto HandPos = AnArchosHand->world.translate;
	auto LeviPos = glm::vec3{
								Levi->GetPositionX(),
								Levi->GetPositionY(),
								Levi->GetPositionZ()
							};
	auto HandPos = glm::vec3{
								AnArchosHand->world.translate.x,
								AnArchosHand->world.translate.y,
								AnArchosHand->world.translate.z
							};
	auto speedVector = glm::normalize(HandPos - LeviPos);
	float distance = glm::distance(LeviPos, HandPos);
	float ArrivalSpeed = distance/_ArrivalTime;
		spdlog::debug("Needed speed: {}", ArrivalSpeed);
//	hkVector4 speedVector;
	while (distance > NearDistance)
	{
		// Yeni konumu güncelle
        LeviPos += speedVector * ArrivalSpeed * 0.033333f; // Örnek bir delta zaman (30 FPS)
		Levi->SetPosition({LeviPos.x, LeviPos.y, LeviPos.z});

        // Yeni mesafeyi hesapla
		distance = glm::distance(LeviPos, HandPos);
		speedVector = glm::normalize(HandPos - LeviPos);

        // Eğer yeni mesafe 100 birimden küçükse döngüden çık
        if (distance <= NearDistance) {
            break;
        }

        // İlerlemenin kontrolü için ekrana bas
        spdlog::info("Mesafe: {}, Konum: ({}, {}, {})", distance, LeviPos.x, LeviPos.y, LeviPos.z);
	}
//	auto a = _Levi->Get
	AnArchos->AddWornItem(_Levi, 1, true, 0, 0);
	spdlog::debug("{} is arrived !!", _Levi->GetName());
	auto LeviAngle = Levi->GetAngle();
}
*/

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
		if (eventTag == Config::ThrowEvent) {
			if (Leviathan::GetThrowState() == Leviathan::ThrowState::kThrowable) {
			//	RE::PlayerCharacter::GetSingleton()->AsActorState()->GetAttackState()
				Leviathan::Throw(false);
			}
			else spdlog::info("Levi is not throwable");
		}
		if (eventTag == Config::CallEvent) {
			if (Leviathan::GetThrowState() == Leviathan::ThrowState::kCanArrive
			 || Leviathan::GetThrowState() == Leviathan::ThrowState::kThrown) Leviathan::Arrive();
		}
		if (eventTag == Config::ChargeLeviEvent) {
	//		Leviathan::Charge(18, 1.8f);	causing crashes
		}

		if (eventTag == Config::SpearThrowEvent) {
			Draupnir::Throw();
		}
		if (eventTag == Config::DraupnirsCallEvent) {
			Draupnir::Call(1.f);
		}
/**/
	auto AnArchos = PlayerCharacter::GetSingleton();
		switch (hash(eventTag.data(), eventTag.size())) {
		// Start phase
		case "weaponDraw"_h:
			WeaponIdentify::WeaponCheck();
			if (WeaponIdentify::LeviathanAxe && AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe) {
				Leviathan::SetThrowState(Leviathan::ThrowState::kThrowable);
				spdlog::debug("Levi is throwable");
				if (Leviathan::SpellCatchLevi && AnArchos->AsMagicTarget()->HasMagicEffect(Leviathan::EffCatchLevi)) {
					AnArchos->RemoveSpell(Leviathan::SpellCatchLevi);
						spdlog::debug("You have the axe catching spell");
				} else {spdlog::debug("You don't have the axe catching spell");}
			}
			break;
/*
		case "FootLeft"_h:
		case "FootRight"_h:
		case "PickNewIdle"_h:
			if (SpellCastTracker::EffectAimButton && AnArchos->AsMagicTarget()->HasMagicEffect(SpellCastTracker::EffectAimButton)) {
				if (APIs::tdm && APIs::tdm->GetDirectionalMovementState()) {
					APIs::result =	APIs::tdm->RequestDisableDirectionalMovement(SKSE::GetPluginHandle());
					if (APIs::result == TDM_API::APIResult::OK || APIs::result == TDM_API::APIResult::AlreadyGiven) spdlog::debug("Request TDM API ok");
					if (APIs::result != TDM_API::APIResult::OK && APIs::result != TDM_API::APIResult::AlreadyGiven) spdlog::debug("Request TDM API not ok");
					AnArchos->NotifyAnimationGraph("PickNewIdle");
					spdlog::info("aiming...");
					break;
				}
			}
			if (SpellCastTracker::EffectAimButton && !AnArchos->AsMagicTarget()->HasMagicEffect(SpellCastTracker::EffectAimButton)) {
				if (APIs::tdm && !APIs::tdm->GetDirectionalMovementState()) {
					APIs::result =	APIs::tdm->ReleaseDisableDirectionalMovement(SKSE::GetPluginHandle());
					if (APIs::result == TDM_API::APIResult::OK || APIs::result == TDM_API::APIResult::AlreadyGiven) spdlog::debug("Release TDM API ok");
					if (APIs::result != TDM_API::APIResult::OK && APIs::result != TDM_API::APIResult::AlreadyGiven) spdlog::debug("Release TDM API not ok");
					AnArchos->NotifyAnimationGraph("PickNewIdle");
					break;
				}
			}
			break;
*/		}
	}

	    return EventChecker::kContinue;
}
/**/
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