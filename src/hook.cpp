#include "hook.h"
#include "util.h"
#include "settings.h"
#include "MainKratosCombat.h"

static std::mutex ThrowCallMutex;

/* //from ersh tdm
void ProjectileHook::ProjectileAimSupport(RE::Projectile* a_this)
	{
		auto projectileNode = a_this->Get3D2();

		// player only, 0x100000 == player
		auto& shooter = a_this->GetProjectileRuntimeData().shooter;
		auto& desiredTarget = a_this->GetProjectileRuntimeData().desiredTarget;
		if (projectileNode && shooter.native_handle() == 0x100000) {
				if (!desiredTarget.get()) {
					auto target = directionalMovementHandler->GetTarget();
					auto targetPoint = directionalMovementHandler->GetTargetPoint();
					if (!target || !targetPoint) {
						return;
					}
					desiredTarget = target;
					directionalMovementHandler->AddProjectileTarget(a_this->GetHandle(), targetPoint);
				
				if (aimType == TargetLockProjectileAimType::kHoming) {
					// homing
					auto targetPoint = directionalMovementHandler->GetProjectileTargetPoint(a_this->GetHandle());
					if (targetPoint) {
						RE::NiPoint3 handPos = targetPoint->world.translate;
						auto& linearVelocity = a_this->GetProjectileRuntimeData().linearVelocity;
						float speed = linearVelocity.Length();
						//if (speed < 1500.f) {
						//	return _GetLinearVelocity(a_this, a_outVelocity);
						//}
						RE::NiPoint3 direction = (handPos - leviPos);
						// normalize direction
						direction.Unitize();
						// rotate
						leviAngle.x = asin(direction.z);
						leviAngle.z = atan2(direction.x, direction.y);
						if (leviAngle.z < 0.0) {
							leviAngle.z += PI;
						}
						if (direction.x < 0.0) {
							leviAngle.z += PI;
						}
						SetRotationMatrix(projectileNode->local.rotate, -direction.x, direction.y, direction.z);
						linearVelocity = direction * speed;
					
					}
				}
			}
		}
	}
*/
void ProjectileHook::GetLinearVelocityMissile(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
{
	_GetLinearVelocityMissile(a_this, a_outVelocity);
	if (ThrowCallMutex.try_lock()) {
		LeviAndDraupnir(a_this);
		ThrowCallMutex.unlock();
	};
}
void ProjectileHook::GetLinearVelocityArrow(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
{
	_GetLinearVelocityArrow(a_this, a_outVelocity);
	if (ThrowCallMutex.try_lock()) {
		LeviAndDraupnir(a_this);
		ThrowCallMutex.unlock();
	};
}
void ProjectileHook::GetLinearVelocityCone(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
{
	_GetLinearVelocityCone(a_this, a_outVelocity);
	if (ThrowCallMutex.try_lock()) {
		LeviAndDraupnir(a_this);
		ThrowCallMutex.unlock();
	};
}
void ProjectileHook::LeviAndDraupnir(RE::Projectile* a_this)
{
	auto projectileNode = a_this->Get3D2();

	auto AnArchos = RE::PlayerCharacter::GetSingleton();
	auto& runtimeData = a_this->GetProjectileRuntimeData();
	auto& shooter = runtimeData.shooter;
	auto& desiredTarget = runtimeData.desiredTarget;

	if (projectileNode && shooter.native_handle() == 0x100000)	// player only, 0x100000 == player
	{
		auto projBase = a_this->GetProjectileBase();
		float livingTime = runtimeData.livingTime;

		if (projBase == Leviathan::LeviProjBaseL || projBase == Leviathan::LeviProjBaseH || projBase == Leviathan::LeviProjBaseA) 
		{
			//identify levi
		//	spdlog::debug("Levi proj flags: {:032b}", runtimeData.flags);	// "10000100001101010000000101010000" kFading, kUnk7, kChainShatter, 
		//	if (runtimeData.flags & (1 << 15)) {
		//		runtimeData.flags &= ~(1 << 15);		//remove the RE::Projectile::Flags::kDestroyAfterHit flag
		//	//	a_this->inGameFormFlags &= (1 << 10);	//RE::TESForm::RecordFlags::kPersistent;
		//		spdlog::debug("Levi proj kDestroyAfterHit flag removed");
		//	}
			if (Leviathan::isAxeStucked) Leviathan::isAxeStucked = false;
			(projBase == Leviathan::LeviProjBaseL) ? Leviathan::LeviathanAxeProjectileL : Leviathan::LeviathanAxeProjectileH = a_this;
			(projBase == Leviathan::LeviProjBaseL) ? Leviathan::LeviathanAxeProjectileH : Leviathan::LeviathanAxeProjectileL = nullptr;
		//	a_this->GetProjectileBase()->SetModel(WeaponIdentify::LeviathanAxe->GetModel());
			Leviathan::ThrowedLevi =  a_this->AsReference1();
			runtimeData.weaponDamage = WeaponIdentify::LeviathanAxe->attackDamage;
			if (Leviathan::throwState == Leviathan::ThrowState::kThrown) Leviathan::SetThrowState(Leviathan::ThrowState::kCanArrive);

			//speed
			auto& vel = runtimeData.linearVelocity;

			//rotation
			float xAxis = Leviathan::LeviProjBaseL ? 0.35f : PI2;		//0.35f = 20 degrees for lateral throw, PI/2 = 90 degrees for vertical throw
			auto linearDir = vel;
			linearDir.Unitize();
			float rot = Config::ThrowRotationSpeed * livingTime;

			//location
			auto targetPoint = WeaponIdentify::RHandBone;
			if (!targetPoint) {spdlog::warn("can't found your hand node for axe call!"); return;}
			auto& handPos	= targetPoint->world.translate;
			auto& leviPos	= a_this->data.location;
			auto& leviAngle	= a_this->data.angle;

			if (!Leviathan::isAxeCalled) {
				vel = linearDir * Config::ThrowSpeed;

				//accelerate between unpleasant hit angles
//				if (leviAngle.z < Config::MinAxeStuckAngle) rot *= 2.f;
//				if (leviAngle.z > Config::MaxAxeStuckAngle) rot *= 2.f;
				//apply rotation
			//	leviAngle.x = asin(linearDir.z);
			//	leviAngle.z -= atan2(linearDir.x, linearDir.y) + rot / 30.f;	//
			//	leviAngle.z -= rot / 30.f;						//main rotation
//	matrix things
//				SetRotationMatrix(projectileNode->local.rotate, -linearDir.x, linearDir.y, linearDir.z);	//	sa * cb, ca * cb, sa
//				NiPoint3 vectorLeviX = {projectileNode->local.rotate.entry[0][0],	//	cosa
//										projectileNode->local.rotate.entry[1][0],	//	sina
//										projectileNode->local.rotate.entry[2][0]};	//	0.0
//				NiPoint3 vectorLeviY = {projectileNode->local.rotate.entry[0][1],	// -sina * cosb
//			/*forward vector of levi*/	projectileNode->local.rotate.entry[1][1],	//	cosa * cosb
//										projectileNode->local.rotate.entry[2][1]};	//	sinb
//				NiPoint3 vectorLeviZ = {projectileNode->local.rotate.entry[0][2],	//	sina * sinb
//			/*upward vector of levi*/	projectileNode->local.rotate.entry[1][2],	// -cosa * sinb
//										projectileNode->local.rotate.entry[2][2]};	//	cosb

				leviAngle.x = asin(linearDir.z);// + xAxis;
				leviAngle.z = atan2(linearDir.x, linearDir.y);// - rot;

//				RE::NiMatrix3 copyMatrix = projectileNode->local.rotate;									//	copy of original rotation matrix
//				SetRotationMatrix(copyMatrix, -linearDir.x, linearDir.y, linearDir.z);						//	set rotation of copied matrix
//				MathUtil::Algebra::RotateMatrixAroundAxisses(copyMatrix, xAxis, 0.f, rot);					//	add rotation
//				projectileNode->local.rotate = copyMatrix;													//	apply rotation
//				projectileNode->world.rotate = copyMatrix;

				float dist = AnArchos->data.location.GetDistance(leviPos);
				Leviathan::leviPosition = leviPos;
				Leviathan::throwedTime = livingTime;
				Leviathan::arrivalSpeed = dist / Config::ArrivalTime;

				if (Leviathan::arrivalSpeed < Config::MinArrivalSpeed) {
					Leviathan::arrivalSpeed = Config::MinArrivalSpeed;
				}
				else if (Leviathan::arrivalSpeed > Config::MaxArrivalSpeed) {
					Leviathan::arrivalSpeed = Config::MaxArrivalSpeed;
				}
			}
			else if (Leviathan::isAxeCalled) {
				if (projBase != Leviathan::LeviProjBaseA) {runtimeData.flags |= (1 << 25); return;}
			//	a_this->GetProjectileBase()->SetModel(WeaponIdentify::LeviathanAxe->GetModel());

				RE::NiPoint3 direction = (handPos - leviPos);						//	direction of the axe to player's hand
				direction.Unitize();												//	normalize direction

				float distance = handPos.GetDistance(leviPos);
				if (distance < Config::CatchingTreshold) {
					if (Leviathan::EffCatchLevi && Leviathan::SpellCatchLevi && !AnArchos->AsMagicTarget()->HasMagicEffect(Leviathan::EffCatchLevi)) {
						AnArchos->AddSpell(Leviathan::SpellCatchLevi);
					} else spdlog::debug("Levi proj spell not added");
					if (Leviathan::GetThrowState() == Leviathan::ThrowState::kCanArrive) Leviathan::SetThrowState(Leviathan::ThrowState::kArrived);
					Leviathan::isAxeCalled = false;
				//	auto handNode = targetPoint->AsNode();
				//	handNode->AttachChild(a_this->Get3D());
					if (WeaponIdentify::LeviathanAxe)
						RE::ActorEquipManager::GetSingleton()->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe, nullptr, 1U, nullptr, false, false, false, true);
					else spdlog::warn("you not have the leviathan axe");
					runtimeData.flags |= (1 << 25);		//set as destroyed, RE::Projectile::Flags::kDestroyed
					spdlog::debug("Levi proj catched");
					return;
				}

				// old speed calculation method
				float passedArrTime = livingTime - Leviathan::throwedTime;
				if (passedArrTime <= 0.f) passedArrTime = livingTime;
				float arrivingTime = Config::ArrivalTime - passedArrTime;
				if (arrivingTime < 0.02f) arrivingTime = 0.02f;
				Leviathan::arrivalSpeed = distance / arrivingTime;
			//	if (Leviathan::arrivalSpeed < Config::MinArrivalSpeed)		Leviathan::arrivalSpeed = Config::MinArrivalSpeed;
				if (Leviathan::arrivalSpeed > Config::MaxArrivalSpeed)	Leviathan::arrivalSpeed = Config::MaxArrivalSpeed;

				float offset = Leviathan::arrivalSpeed * Config::ArrivalRoadCurveMagnitude;
			//	if (distance < 240.f) {
			//		Leviathan::arrivalSpeed *= 0.9f;	//	slows down while getting close
			//		offset *= 0.9f;						//	turns direction to target while getting close
			//	}

				// new method
				RE::NiPoint3 crossDir = direction.UnitCross({-0.5f, 0.f, -1.f});
				vel = (direction * Leviathan::arrivalSpeed) + (crossDir * offset);
					spdlog::debug("arriving speed: {}", vel.Length());
				leviAngle.x = asin(direction.z);
				leviAngle.z = atan2(direction.x, direction.y);

				// old direction calculating method
			//	RE::NiPoint3 midPoint = (handPos + leviPos) / 2;
			//	RE::NiPoint3 curvePoint = midPoint + crossDir * offset;
			//	if (offset < 90.f) {
			//		curvePoint = handPos;
			//	}
			//		spdlog::debug("curvepoint dist: {} midpoint dist: {}", curvePoint.GetDistance(handPos), midPoint.GetDistance(handPos));
			//	RE::NiPoint3 curvyDir = (curvePoint - leviPos);
			//	curvyDir.Unitize();
				// rotate
			//	float arrivingAngle = PI2 * passedArrTime / Config::ArrivalTime;
			//	leviAngle.x = asin(curvyDir.z);
			//	leviAngle.z = atan2(curvyDir.x, curvyDir.y);
			//	if (leviAngle.y < PI2) {
			//		leviAngle.y -= 0.0017;
			//	}
			//	if (leviAngle.x < PI) {
			//		leviAngle.x -= 0.00017;
			//	}
			//	if (leviAngle.z < 0.0) {
			//		leviAngle.z += PI;
			//	}
			//	if (direction.x < 0.0) {
			//		leviAngle.z += PI;
			//	}
			//	float rot = Config::ArrivalRotationSpeed * livingTime;
			//	RE::NiMatrix3 copyMatrix = projectileNode->local.rotate;
			//	SetRotationMatrix(copyMatrix, -curvyDir.x, curvyDir.y, curvyDir.z);
			//	MathUtil::Algebra::RotateMatrixAroundAxisses(copyMatrix, -PI2, PI, rot);
			//	projectileNode->local.rotate = copyMatrix;
			//
			//	vel = curvyDir * 1200.f;
			}
		}
		else if (projBase == Draupnir::DraupnirSpearProjBaseL) 
		{	//WIP
		//	if (runtimeData.flags & (1 << 15)) runtimeData.flags &= ~(1 << 15);		//remove the RE::Projectile::Flags::kDestroyAfterHit flag
			if (livingTime > 0.3f) return;				//	prevents duplicates
			auto lastFormID = a_this->GetFormID();
			int nextIndex = -1;
			int nextNextIndex = -1;
			for (int i = 0; i <= Config::DraupnirSpearCount; i++) {
				if (Draupnir::DraupnirSpearProjectiles[i] && Draupnir::DraupnirSpearProjectiles[i]->GetFormID() == lastFormID) return;		//	prevents duplicates
			}

			for (int i = 0; i <= Config::DraupnirSpearCount; i++) {
				if (Draupnir::DraupnirSpearProjectiles[i] == nullptr) {
					nextIndex = i;
					if (nextIndex == Config::DraupnirSpearCount) nextNextIndex = 0;
					else nextNextIndex = nextIndex + 1;												//	every spear opens area for next spear
					break;
				}
			}
			if (nextIndex == -1) return;
			if (nextNextIndex == -1) {
				nextNextIndex = 0;
					spdlog::debug("{}. VERY WEIRD {:08x}", nextIndex, lastFormID);
			}
			if (Draupnir::DraupnirSpearProjectiles[nextNextIndex] != nullptr) Draupnir::DraupnirSpearProjectiles[nextNextIndex] = nullptr;

			Draupnir::DraupnirSpearProjectiles[nextIndex] = a_this;
				spdlog::debug("{}. Draupnir spear throwed, ID: {:08x}", (nextIndex + 1), lastFormID);
		}
	//	else 
	//	{
	//		return;
	//		auto x = a_this->GetAngleX();	//changes with self z axis rotation, good for light axe throwing rotation
	//		auto y = a_this->GetAngleY();	//allways 0		y is must be moving axis
	//		auto z = a_this->GetAngleZ();	//changes with looking angle (left - right - front...)
	//		spdlog::debug("{:08x}'s rotation is x:{} y:{} z:{}", a_this->GetFormID(), x, y, z);
	//		//We don't care about the other projectiles
	//	}
	}
}
void ProjectileHook::GetCollisionFlame(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	_GetCollisionFlame(a_this, a_AllCdPointCollector);
}
void ProjectileHook::GetCollisionMissile(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (LeviAndDraupnirHit(a_this, a_AllCdPointCollector)) {
		spdlog::debug("levi hit type changed from missile to beam type");
		
		return;//	GetCollisionBeam(a_this, a_AllCdPointCollector);
	}

	_GetCollisionMissile(a_this, a_AllCdPointCollector);
}
void ProjectileHook::GetCollisionArrow(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (LeviAndDraupnirHit(a_this, a_AllCdPointCollector)) {
		spdlog::debug("levi hit type changed from arrow to beam type");

		return;//	GetCollisionBeam(a_this, a_AllCdPointCollector);;
	}

	_GetCollisionArrow(a_this, a_AllCdPointCollector);
}

bool ProjectileHook::LeviAndDraupnirHit(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	auto projBase = a_this->GetProjectileBase();
	if (projBase && projBase == Leviathan::LeviProjBaseA || projBase == Leviathan::LeviProjBaseL || projBase == Leviathan::LeviProjBaseH) {
		if (Leviathan::isAxeCalled) {//return false;

			for (auto& point : a_AllCdPointCollector->hits) {
				auto collidableA = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableA);
				auto collidableB = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

				if (collidableA) {
					auto hitposA = collidableA->data.location;
					a_this->data.location = hitposA;
					auto victim = collidableA->As<RE::Actor>();
					if (victim) {
						auto damage = a_this->GetProjectileRuntimeData().weaponDamage;
						victim->AsActorValueOwner()->ModActorValue(RE::ActorValue::kHealth, -damage);
						spdlog::info("levi caused {} damage to {} while arriving!!", damage, victim->GetName());
						return true;
					}
					spdlog::debug("levi hitted somewhere while arriving");
					return true;
				}
				if (collidableB) {
					auto hitposB = collidableB->data.location;
					a_this->data.location = hitposB;
					auto victim = collidableB->As<RE::Actor>();
					if (victim) {
						auto damage = a_this->GetProjectileRuntimeData().weaponDamage;
						victim->AsActorValueOwner()->ModActorValue(RE::ActorValue::kHealth, -damage);
						spdlog::info("levi caused {} damage to {} while arriving!!", damage, victim->GetName());
						return true;
					}
					spdlog::debug("levi hitted somewhere while arriving");
					return true;
				}
			}
			return true;
		}

		Leviathan::isAxeStucked = true;
		Leviathan::ThrowedLevi2 = a_this->Get3D();
		Leviathan::ThrowedLevi1 = a_this->loadedData->data3D.get();

		if (Leviathan::throwState == Leviathan::ThrowState::kThrown) Leviathan::SetThrowState(Leviathan::ThrowState::kCanArrive);
	}
//	else if (projBase && projBase == Draupnir::DraupnirSpearProjBaseL) a_this->GetProjectileRuntimeData().flags &= ~(1 << 2);

	return false;
}
/**/
uint32_t* ProjectileHook::Launch(uint32_t* handle, RE::Projectile::LaunchData* ldata)
{
	auto fenix31415 = ldata->shooter;
	if (fenix31415 && fenix31415->IsPlayerRef()) {
		if (ldata->projectileBase == Leviathan::LeviProjBaseA) {
			ldata->useOrigin = true;
			ldata->origin = Leviathan::leviPosition;
			if (Leviathan::ThrowedLevi2) {
				if (Leviathan::ThrowedLevi2->GetFlags() & RE::NiAVObject::Flag::kDisplayObject) {
					ldata->origin = Leviathan::ThrowedLevi2->world.translate;
					spdlog::debug("we get origin from throwed levi 2!!");
				}
			}
			else if (Leviathan::ThrowedLevi1) {
				if (Leviathan::ThrowedLevi1->GetFlags() & RE::NiAVObject::Flag::kDisplayObject) {
					ldata->origin = Leviathan::ThrowedLevi1->world.translate;
					spdlog::debug("we get origin from throwed levi 1!!");
				}
			} 
			else spdlog::debug("can't found levi for calling!");
		}
	}

	return _Launch(handle, ldata);
}
