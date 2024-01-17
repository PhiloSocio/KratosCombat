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
void ProjectileHook::GetLinearVelocityProj(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
{
	_GetLinearVelocityProj(a_this, a_outVelocity);
	if (ThrowCallMutex.try_lock()) {
		LeviAndDraupnir(a_this);
		ThrowCallMutex.unlock();
	};
}
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

	auto& runtimeData = a_this->GetProjectileRuntimeData();
	auto& shooter = runtimeData.shooter;

	if (projectileNode && shooter.native_handle() == 0x100000)	// player only, 0x100000 == player
	{
		auto projBase = a_this->GetProjectileBase();
		float livingTime = runtimeData.livingTime;
		auto AnArchos = RE::PlayerCharacter::GetSingleton();

		if (WeaponIdentify::IsRelic(projBase, true)) 
		{
		//	auto animatedBone = projectileNode->GetObjectByName("Slash");
		//	auto leviModel = (WeaponIdentify::LeviathanAxe->AsReference1() ? WeaponIdentify::LeviathanAxe->AsReference1()->Get3D() : nullptr);
		//	if (animatedBone && leviModel) {
		//		auto animatedNode = animatedBone->AsNode();
		//		auto leviNode = leviModel->AsNode();
		//		if (animatedNode && leviNode) {
		//			animatedNode->AttachChild(leviNode, false);
		//			spdlog::debug("levi model changed!");
		//		} else spdlog::debug("animated node or levinode null");
		//	} else spdlog::debug("animated bone or leviModel null");
			//identify levi
		//	spdlog::debug("Levi proj flags: {:032b}", runtimeData.flags);	// "10000100001101010000000101010000" kFading, kUnk7, kChainShatter, 
		//	if (runtimeData.flags & (1 << 15)) {
		//		runtimeData.flags &= ~(1 << 15);		//remove the RE::Projectile::Flags::kDestroyAfterHit flag
		//	//	a_this->inGameFormFlags &= (1 << 10);	//RE::TESForm::RecordFlags::kPersistent;
		//		spdlog::debug("Levi proj kDestroyAfterHit flag removed");
		//	}
			//location
			auto targetPoint = WeaponIdentify::RHandBone;
			if (!targetPoint) {spdlog::warn("can't found your hand node for axe call!!"); return;}
			auto& handPos	= targetPoint->world.translate;
			auto& leviPos	= a_this->data.location;
			auto& leviAngle	= a_this->data.angle;

			if (Leviathan::isAxeStucked) Leviathan::isAxeStucked = false;
			(projBase == Leviathan::LeviProjBaseL ? Leviathan::LeviathanAxeProjectileL : Leviathan::LeviathanAxeProjectileH) = a_this;
			(projBase == Leviathan::LeviProjBaseL ? Leviathan::LeviathanAxeProjectileH : Leviathan::LeviathanAxeProjectileL) = nullptr;
		//	a_this->GetProjectileBase()->SetModel(WeaponIdentify::LeviathanAxe->GetModel());
		//	runtimeData.weaponDamage = WeaponIdentify::LeviathanAxe->attackDamage;
			if (Leviathan::throwState == Leviathan::ThrowState::kThrown) Leviathan::SetThrowState(Leviathan::ThrowState::kCanArrive);

			//rotation
		//	float yAxis = (Leviathan::LeviProjBaseL ? 0.35f : PI2);		//0.35f = 20 degrees for lateral throw, PI/2 = 90 degrees for vertical throw

			//speed
			auto& vel = runtimeData.linearVelocity;

			auto linearDir = vel;
			linearDir.Unitize();


			if (!Leviathan::isAxeCalled) {
				if (a_this != Leviathan::LastLeviProjectile)	Leviathan::LastLeviProjectile = a_this;
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

			//	float rot = Config::ThrowRotationSpeed * livingTime;
				leviAngle.x = asin(linearDir.z);
			//	leviAngle.y = yAxis;
			//	leviAngle.z = rot;

//				RE::NiMatrix3 copyMatrix = projectileNode->local.rotate;									//	copy of original rotation matrix
//				SetRotationMatrix(copyMatrix, -linearDir.x, linearDir.y, linearDir.z);						//	set rotation of copied matrix
//				MathUtil::Algebra::RotateMatrixAroundAxisses(copyMatrix, xAxis, 0.f, rot);					//	add rotation
//				projectileNode->local.rotate = copyMatrix;													//	apply rotation
//				projectileNode->world.rotate = copyMatrix;

				float dist = AnArchos->data.location.GetDistance(leviPos);
				Leviathan::leviPosition = leviPos;
				Leviathan::throwedTime = livingTime;
				Leviathan::arrivalSpeed = dist / Config::ArrivalTime;

				if		(Leviathan::arrivalSpeed < Config::MinArrivalSpeed)	Leviathan::arrivalSpeed = Config::MinArrivalSpeed;
				else if	(Leviathan::arrivalSpeed > Config::MaxArrivalSpeed)	Leviathan::arrivalSpeed = Config::MaxArrivalSpeed;
			}
			else if (Leviathan::isAxeCalled) {
				if (projBase != Leviathan::LeviProjBaseA) {runtimeData.flags |= (1 << 25); return;}
				if (Leviathan::LeviathanAxeProjectileA != a_this) Leviathan::LeviathanAxeProjectileA = a_this;
				if (Leviathan::throwState == Leviathan::ThrowState::kCanArrive) Leviathan::SetThrowState(Leviathan::ThrowState::kArriving);
			//	a_this->GetProjectileBase()->SetModel(WeaponIdentify::LeviathanAxe->GetModel());

				RE::NiPoint3 direction = (handPos - leviPos);						//	direction of the axe to player's hand
				direction.Unitize();												//	normalize direction

				float passedArrTime = livingTime - Leviathan::throwedTime;
				if (passedArrTime < 0.f) passedArrTime = livingTime;
				float arrivingTime = Config::ArrivalTime - passedArrTime;
				if (arrivingTime < 0.01f) arrivingTime = 0.01f;

				float distance = handPos.GetDistance(leviPos);
				if (distance < Config::CatchingTreshold) {
					if (Leviathan::EffCatchLevi && Leviathan::SpellCatchLevi && !AnArchos->AsMagicTarget()->HasMagicEffect(Leviathan::EffCatchLevi)) {
						AnArchos->AddSpell(Leviathan::SpellCatchLevi);
					} else spdlog::debug("Levi proj spell not added");

					if (Leviathan::throwState == Leviathan::ThrowState::kArriving) Leviathan::SetThrowState(Leviathan::ThrowState::kArrived);
					Leviathan::isAxeCalled = false;

					if (WeaponIdentify::LeviathanAxe)
						RE::ActorEquipManager::GetSingleton()->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, false, false, false, true);
					else spdlog::warn("you not have the leviathan axe");
					runtimeData.flags |= (1 << 25);		//set as destroyed, RE::Projectile::Flags::kDestroyed
					spdlog::debug("Levi proj catched");
					return;
				}

				// speed calculation
				float arrSpeed = distance / arrivingTime;
				if		(arrSpeed < Leviathan::arrivalSpeed)	arrSpeed = Leviathan::arrivalSpeed;
				else if	(arrSpeed > Config::MaxArrivalSpeed)	arrSpeed = Leviathan::arrivalSpeed;

				float sinAngle	= Config::ArrivalRoadCurveMagnitude;
				float distMoved	= runtimeData.distanceMoved;
				float halfWay	= distance - distMoved;
				float tertiaWay	= 2.f * distance - distMoved;
				float quartWay	= 3.f * distance - distMoved;
				float sexWay	= 5.f * distance - distMoved;	//	sexWay = 0 means the axe finished 5/6 of the way
				if		(sexWay		< 0.f)	{/**/ arrSpeed *= 0.3f; sinAngle = 0.f;}
				else if	(quartWay	< 0.f)	{/**/ arrSpeed *= 0.7f; sinAngle *= 0.4f;}
				else if	(tertiaWay	< 0.f)	{/**/ arrSpeed *= 0.8f; sinAngle *= 0.7f;}
				else if	(halfWay	< 0.f)	{/**/ arrSpeed *= 0.9f; sinAngle *= 0.9f;}				//	turns direction to target while getting close

				if	(arrSpeed < Config::MinArrivalSpeed)	arrSpeed = Config::MinArrivalSpeed;

				float offset = arrSpeed * sinAngle;
				if (offset > 3200.f) offset = 3200.f;

		//		auto cone = a_this->As<RE::ConeProjectile>();
		//		if (cone) {
		//		//	auto& coneRTD = cone->GetConeRuntimeData();
		//			cone->GetConeRuntimeData().coneAngleTangent = 120 / (distMoved + 1);
		//			}

				// new direction calculating method
				RE::NiPoint3 crossDir = direction.UnitCross(downVec);
				vel = (direction * arrSpeed) + (crossDir * offset);
			//		spdlog::debug("distance: {}", distance);
				RE::NiPoint3 curvyDir = vel;
				curvyDir.Unitize();
				leviAngle.x = asin(curvyDir.z);
				leviAngle.z = atan2(curvyDir.x, curvyDir.y);
			//	MathUtil::Algebra::SetRotationMatrix(projectileNode->local.rotate, -curvyDir.x, curvyDir.y, curvyDir.z);

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
		else if (projBase == Draupnir::DraupnirSpearProjBaseL) {
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
		else if (projBase == Draupnir::DraupnirsCallProjBaseL) {
			int i = 0;
			for (auto firstBone : Draupnir::DraupnirSpearHitBones) {
				if (firstBone) break;
				i++;
			}
			auto targetPoint = Draupnir::DraupnirSpearHitBones[i];
			auto& targetPos	= targetPoint->world.translate;
			auto& leviPos	= a_this->data.location;
			auto& vel = runtimeData.linearVelocity;
			auto direction = (targetPos - leviPos);
			auto speed = vel.Length();
			direction.Unitize();
			vel = direction * speed;
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
	if (LeviAndDraupnirHit(a_this, a_AllCdPointCollector)) return;
	_GetCollisionFlame(a_this, a_AllCdPointCollector);
}
void ProjectileHook::GetCollisionCone(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (LeviAndDraupnirHit(a_this, a_AllCdPointCollector)) return;
	_GetCollisionCone(a_this, a_AllCdPointCollector);
}
void ProjectileHook::GetCollisionMissile(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (LeviAndDraupnirHit(a_this, a_AllCdPointCollector)) return;
	_GetCollisionMissile(a_this, a_AllCdPointCollector);
}
void ProjectileHook::GetCollisionArrow(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (LeviAndDraupnirHit(a_this, a_AllCdPointCollector)) return;
	_GetCollisionArrow(a_this, a_AllCdPointCollector);
}
void ProjectileHook::GetCollisionProjectile(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (LeviAndDraupnirHit(a_this, a_AllCdPointCollector)) return;
	_GetCollisionProjectile(a_this, a_AllCdPointCollector);
}

inline bool ProjectileHook::LeviAndDraupnirHit(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	auto projBase = a_this->GetProjectileBase();
	if (projBase && WeaponIdentify::IsRelic(projBase, true)) {
		if (Leviathan::isAxeCalled && Config::DontDamageWhileArrive)	return true;

	/*	auto model = a_this->Get3D();
		if (model) {
			auto controllers = model->GetControllers();
			if (controllers) {
				auto manager = controllers->AsNiControllerManager();
				for (auto sequence : manager->activeSequences) {
					spdlog::debug("active sequence is {}", sequence->name);
					auto endSequence = manager->GetSequenceByName("mEnd");
					if (endSequence) {
						sequence = endSequence;
						spdlog::debug("end sequence is {}", endSequence->name);
					}
				}
			}
		}
	*/	for (auto& point : a_AllCdPointCollector->hits) {
			auto ourProj 	= RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableA);
			auto target 	= RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

			if (ourProj) {
			//	if (projBase == Leviathan::LeviProjBaseL) a_this->data.angle.z = Config::MaxAxeStuckAngle;
			//	else a_this->data.angle.y = Config::MaxAxeStuckAngle;
			}
			if (target && !target->AsProjectile()) {
				if (!Leviathan::isAxeStucked) Leviathan::isAxeStucked = true;
			//	spdlog::debug("levi hitted to {}!!", target->GetName());
				if (target->formType == RE::FormType::ActorCharacter) {
					auto victim = target->As<RE::Actor>();
					if (victim) {
						if (a_this->IsMissileProjectile()) Leviathan::leviStuckedActor = victim;
					//	auto damage = a_this->GetProjectileRuntimeData().weaponDamage;
					//	victim->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -damage);
					//	spdlog::debug("levi caused {} damage to {} {}", damage, victim->GetName(), (a_this->IsMissileProjectile() ?  "!!" : "while arriving!!"));
					}
				}
			}
		}

		if (Leviathan::throwState == Leviathan::ThrowState::kThrown) Leviathan::SetThrowState(Leviathan::ThrowState::kCanArrive);
		
		return false;
	}
/*
	if (projBase == Draupnir::DraupnirsCallProjBaseL) {
		for (auto& point : a_AllCdPointCollector->hits) {
			auto target 	= RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

			if (target && !target->AsProjectile()) {
				if (target->formType == RE::FormType::ActorCharacter) {
					auto victim = target->As<RE::Actor>();
					if (victim) {
						for (auto validTarget : Draupnir::DraupnirSpearHitActors) {
							if (validTarget && validTarget == victim) {
								victim->RemoveExtraArrows3D();
								return false;
							}
						}
					}
				}
			}
		}
		return true;
	}
*/
	return false;
}
/*
uint32_t* ProjectileHook::Launch(uint32_t* handle, RE::Projectile::LaunchData* ldata)
{
	if (!ldata) return _Launch(handle, ldata);

	auto fenix31415 = ldata->shooter;
	if (fenix31415 && fenix31415->IsPlayerRef()) {
		auto projBase = ldata->projectileBase;
		if (WeaponIdentify::IsRelic(projBase)) {
			if (projBase == Leviathan::LeviProjBaseA) {
				ldata->useOrigin = true;
				ldata->origin = Leviathan::leviPosition;
				auto stuckedLevi =	Leviathan::LastLeviProjectile ? Leviathan::LastLeviProjectile : nullptr;
				if (!stuckedLevi) stuckedLevi =	(Leviathan::LeviathanAxeProjectileL ? Leviathan::LeviathanAxeProjectileL : 
												(Leviathan::LeviathanAxeProjectileH ? Leviathan::LeviathanAxeProjectileH : nullptr));
				if (stuckedLevi)	ldata->origin = stuckedLevi->data.location;
				else spdlog::debug("we cant get leviathan's stucked proj");

				if (Leviathan::leviStuckedBone) {
					ldata->origin = Leviathan::leviStuckedBone->world.translate;
				//	auto attachedLevi = Leviathan::leviStuckedBone->GetObjectByName(stuckedLevi->GetName());
				//	if (attachedLevi) attachedLevi->GetFlags() &= RE::NiAVObject::Flag::kDisplayObject;		//	ctd
				//	else spdlog::debug("we cant found attached levi");
					Leviathan::leviStuckedBone = nullptr;
				} else spdlog::debug("levi not stucked any bone");

			//	fenix31415->GetWorldspace()->worldMapData.usableWidth;

				if (Leviathan::leviStuckedActor) {
					Leviathan::leviStuckedActor->RemoveExtraArrows3D();
					spdlog::debug("levi stucked actor's extra arrows removed");
					Leviathan::leviStuckedActor = nullptr;
				//	RE::BaseExtraList extraList;
				//	RE::ExtraAttachedArrows3D* attachedArrow;
				//	auto targetData = Leviathan::leviStuckedActor->extraList.Remove<RE::ExtraAttachedArrows3D>(attachedArrow);
				//	for (auto attachedLevi : attachedArrow->data) {
				//		if (WeaponIdentify::IsRelic(attachedLevi.source)) {
				//			attachedLevi.arrow3D.get()->DeleteThis();
				//		}
				//	}
				} else spdlog::debug("levi not stucked anybody");

				Leviathan::SetThrowState(Leviathan::ThrowState::kArriving);
			}
			if (!ldata->weaponSource && WeaponIdentify::LeviathanAxe && WeaponIdentify::DraupnirSpear) 
				ldata->weaponSource = (WeaponIdentify::IsRelic(projBase, true) ? WeaponIdentify::LeviathanAxe : WeaponIdentify::DraupnirSpear);
			if (ldata->weaponSource) {
				if (!ldata->enchantItem) {
					if (ldata->weaponSource->formEnchanting && ldata->weaponSource->formEnchanting->data.baseEnchantment)
						ldata->enchantItem = ldata->weaponSource->formEnchanting->data.baseEnchantment;
				}
			}
		}
	}

	return _Launch(handle, ldata);
}
*/
/**/
RE::Projectile::ImpactData* ProjectileHook::GetArrowImpactData(RE::ArrowProjectile *proj, RE::TESObjectREFR *a2, RE::NiPoint3 *a3, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, char a7)
{
	auto impactData = _GetArrowImpactData(proj, a2, a3, a_velocity, a_collidable, a6, a7);
	if (proj && impactData) {
		auto projBase = proj->GetProjectileBase();
		if (projBase == Leviathan::LeviProjBaseA) {
			spdlog::debug("{} is impacted!", projBase->GetName());
		//	proj->GetMissileRuntimeData().impactResult = RE::ImpactResult::kNone;
		//	impactData->impactResult = RE::ImpactResult::kNone;
		//	auto AnArchos = RE::PlayerCharacter::GetSingleton();
		//	AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)
		//			->CastSpellImmediate(Leviathan::SpellLeviProjA, false, nullptr, 1.f, false, WeaponIdentify::LeviathanAxe->attackDamage, AnArchos);
		}
		else if (WeaponIdentify::IsRelic(projBase, true)) {
			Leviathan::isAxeStucked = true;
			proj->GetMissileRuntimeData().impactResult = RE::ImpactResult::kStick;
			impactData->impactResult = RE::ImpactResult::kStick;
			Leviathan::leviStuckedBone = impactData->damageRootNode;
			spdlog::debug("{} is sticked to {}!", projBase->GetName(), Leviathan::leviStuckedBone ? Leviathan::leviStuckedBone->name : "null");
		//	if (projBase == Leviathan::LeviProjBaseL) proj->data.angle.z = Config::MaxAxeStuckAngle;
		//	else proj->data.angle.y = Config::MaxAxeStuckAngle;
			spdlog::debug("hit angle = [{}, {}, {}]", proj->data.angle.x, proj->data.angle.y, proj->data.angle.z);
		}
		else if (WeaponIdentify::IsRelic(projBase, false, true)) {
			proj->GetMissileRuntimeData().impactResult = RE::ImpactResult::kStick;
			impactData->impactResult = RE::ImpactResult::kStick;
			auto hitBone = impactData->damageRootNode;
			spdlog::debug("{} is sticked!", projBase->GetName());
			if (a2 && a2->formType == RE::FormType::ActorCharacter) {
				auto target = a2->As<RE::Actor>();
				if (target && hitBone) {
				//	spdlog::debug("draupnir attached to {} of {}!", hitBone->name, target->GetName());
					int nthDraupnir = 0;
					for (auto draupnir : Draupnir::DraupnirSpearProjectiles) {
						if (draupnir && draupnir == proj) {
							Draupnir::DraupnirSpearHitBones[nthDraupnir] = hitBone;
							Draupnir::DraupnirSpearHitActors[nthDraupnir] = target;
							spdlog::debug("{}. draupnir attached to {} of {}!",nthDraupnir , hitBone->name, target->GetName());
						}
						nthDraupnir++;
					}
				}
				else spdlog::debug("draupnir attached to null");
			}
			
			
		}
	}
	return impactData;
}
void ProjectileHook::UpdateMovingCone(RE::ConeProjectile* proj, float dtime)
{
	_UpdateMovingCone(proj, dtime);
	{
		float DistanceMoved = proj->GetProjectileRuntimeData().distanceMoved + 1.f;
		const float cone_len = 269.f;
		proj->GetConeRuntimeData().coneAngleTangent = cone_len / DistanceMoved;
	}
}

/*
bool ProjectileHook::GetKillOnCollisionMissile(RE::MissileProjectile* a_this)
{
	return !_GetKillOnCollisionMissile(a_this);
}
bool ProjectileHook::GetKillOnCollisionArrow(RE::ArrowProjectile* a_this)
{
	return !_GetKillOnCollisionArrow(a_this);
}

void ProjectileHook::ReLaunch(RE::Projectile* a_this, RE::NiPoint3* a_dV)
{
	using func_t = decltype(&ReLaunch);
	REL::Relocation<func_t> func{ RELOCATION_ID(43007, 0) };
	return func(a_this, a_dV);
}

uint32_t* ProjectileHook::_Launch(uint32_t* handle, RE::Projectile::LaunchData* ldata)
{
	using func_t = decltype(&Launch);
	REL::Relocation<func_t> func{ RELOCATION_ID(33672, 34452) };
	return func(handle, ldata);
}
void ProjectileHook::_UpdateMovingCone(RE::ConeProjectile* proj, float dtime)
{
	using func_t = decltype(&_UpdateMovingCone);
	REL::Relocation<func_t> func{ RELOCATION_ID(42624, 43789) };
	return func(proj, dtime);
}
*/