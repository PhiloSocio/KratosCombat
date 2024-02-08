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
	const auto& shooter = runtimeData.shooter;

	if (shooter.native_handle() == 0x100000)	// player only, 0x100000 == player
	{
		if (!projectileNode) {spdlog::warn("projectile's 3d not loaded"); return;}
		const auto projBase = a_this->GetProjectileBase();
		const float livingTime = runtimeData.livingTime;
		const auto AnArchos = RE::PlayerCharacter::GetSingleton();

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
			const auto targetPoint = WeaponIdentify::WeaponBone;
			if (!targetPoint) {spdlog::warn("can't found your hand node for axe call!!"); return;}
			auto& handPos	= targetPoint->world.translate;
			auto& leviPos	= a_this->data.location;
			auto& leviAngle	= a_this->data.angle;

			if (Leviathan::isAxeStucked) Leviathan::isAxeStucked = false;
			if (a_this != Leviathan::LastLeviProjectile)	Leviathan::LastLeviProjectile = a_this;
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
				float speed = Config::ThrowSpeed;
			//	speed *= (*g_deltaTimeRealTime / *g_deltaTime);	//	for not be effected by slow motion/fast motion
				vel = linearDir * speed;

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

				Leviathan::leviPosition = leviPos;
				Leviathan::throwedTime = livingTime;
			}
			else if (Leviathan::isAxeCalled) {
				if (projBase != Leviathan::LeviProjBaseA) {
					runtimeData.flags |= (1 << 25); 
				//	spdlog::debug("[HOOK] levi destroyed before call"); 
					return;
				}
				if (Leviathan::LeviathanAxeProjectileA != a_this) Leviathan::LeviathanAxeProjectileA = a_this;
				if (Leviathan::throwState == Leviathan::ThrowState::kCanArrive) Leviathan::SetThrowState(Leviathan::ThrowState::kArriving);
			//	a_this->GetProjectileBase()->SetModel(WeaponIdentify::LeviathanAxe->GetModel());

				RE::NiPoint3 direction = (handPos - leviPos);						//	direction of the axe to player's hand
				direction.Unitize();												//	normalize direction

			//	float passedArrTime = livingTime - Leviathan::throwedTime;
			//	if (passedArrTime < 0.f) passedArrTime = livingTime;
				float arrivingTime = Config::ArrivalTime - livingTime;//passedArrTime;
				if (arrivingTime < *g_deltaTime * 2.f) arrivingTime = *g_deltaTime * 2.f;

				const float distance = handPos.GetDistance(leviPos);
				if (distance < Config::CatchingTreshold) {
					if (Leviathan::throwState == Leviathan::ThrowState::kArriving) Leviathan::SetThrowState(Leviathan::ThrowState::kArrived);

					Leviathan::Catch(a_this, AnArchos);

					spdlog::debug("Levi proj catched");
				}

			//	speed calculation
				float arrSpeed = distance / arrivingTime;
				if		(arrSpeed < Config::MinArrivalSpeed)	arrSpeed = Config::MinArrivalSpeed;
				else if	(arrSpeed > Config::MaxArrivalSpeed)	arrSpeed = Config::MaxArrivalSpeed;

				if (distance <= (*g_deltaTime * arrSpeed)) {
						if (distance >= Config::CatchingTreshold) {
						if (Leviathan::throwState == Leviathan::ThrowState::kArriving) Leviathan::SetThrowState(Leviathan::ThrowState::kArrived);

						Leviathan::Catch(a_this, AnArchos);

						spdlog::debug("Levi proj catched");
					}
					runtimeData.flags |= (1 << 25);
				//	spdlog::debug("levi is catched!! dist {}, speed {}, dt {}, dtRT {}", distance, arrSpeed, *g_deltaTime, *g_deltaTimeRealTime);
				}

				float sinAngle	= Config::ArrivalRoadCurveMagnitude;
		//		float distMoved	= runtimeData.distanceMoved;
		//		float halfWay	= distance - distMoved;
		//		float tertiaWay	= 2.f * distance - distMoved;
		//		float quartWay	= 3.f * distance - distMoved;
		//		float sexWay	= 5.f * distance - distMoved;	//	sexWay = 0 means the axe finished 5/6 of the way
		//		if		(sexWay		< 0.f)	{/**/ arrSpeed *= 0.3f; sinAngle = 0.f;}
		//		else if	(quartWay	< 0.f)	{/**/ arrSpeed *= 0.7f; sinAngle *= 0.4f;}
		//		else if	(tertiaWay	< 0.f)	{/**/ arrSpeed *= 0.8f; sinAngle *= 0.7f;}
		//		else if	(halfWay	< 0.f)	{/**/ arrSpeed *= 0.9f; sinAngle *= 0.9f;}				//	turns direction to target while getting close
		//
		//		if	(arrSpeed < Config::MinArrivalSpeed)	arrSpeed = Config::MinArrivalSpeed;

				const float currSpeed = vel.Length() + 1.f;
				const float timeToArrive = distance / currSpeed;
			//	arrSpeed *= (*g_deltaTimeRealTime / *g_deltaTime);		//	for not be effected by slow motion/fast motion *but causing very buggy situations
				float curveMult = timeToArrive / Config::ArrivalTime;	//	for reduce the offset while getting close
				if (curveMult > 1.6f) curveMult = 1.6f;
				float offset = arrSpeed * sinAngle * curveMult;
			//	if	(offset	> 3600.f)	offset = 3600.f;

		//		auto cone = a_this->As<RE::ConeProjectile>();
		//		if (cone) {
		//		//	auto& coneRTD = cone->GetConeRuntimeData();
		//			cone->GetConeRuntimeData().coneAngleTangent = 120 / (distMoved + 1);
		//			}

				// new direction calculating method
				RE::NiPoint3 crossDir = direction.UnitCross(downVec);
				vel = (direction * arrSpeed) + (crossDir * offset);
			//		spdlog::debug("distance: {}, passed arriving time: {}", distance, passedArrTime);
				RE::NiPoint3 curvyDir = vel;
				curvyDir.Unitize();
				leviAngle.x = asin(curvyDir.z);
				leviAngle.z = atan2(curvyDir.x, curvyDir.y);
				MathUtil::Algebra::SetRotationMatrix(projectileNode->local.rotate, -curvyDir.x, curvyDir.y, curvyDir.z);

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
			const auto lastFormID = a_this->GetFormID();
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
		// experimental
		else if (projBase == Draupnir::DraupnirsCallProjBaseL) {
			int i = 0;
			for (const auto firstBone : Draupnir::DraupnirSpearHitBones) {
				if (firstBone) break;
				i++;
			}
			const auto targetPoint = Draupnir::DraupnirSpearHitBones[i];
			const auto& targetPos	= targetPoint->world.translate;
			const auto& leviPos	= a_this->data.location;
			auto& vel = runtimeData.linearVelocity;
			auto direction = (targetPos - leviPos);
			const auto speed = 1800.f; // vel.Length();
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
	const auto projBase = a_this->GetProjectileBase();
	if (projBase && WeaponIdentify::IsRelic(projBase)) {
		
		const auto& rtData = a_this->GetProjectileRuntimeData();
		RE::Actor* shooter = nullptr;
		if (rtData.shooter && rtData.shooter.get() && rtData.shooter.get().get())	shooter = rtData.shooter.get().get()->As<RE::Actor>();
	
		if (projBase && WeaponIdentify::IsRelic(projBase, true)) {
			if (Leviathan::isAxeCalled && Config::DontDamageWhileArrive)	return true;

			const bool isArriving = projBase == Leviathan::LeviProjBaseA;
			bool isTargetActor = false;
			bool isSameTarget = false;
	/**/
			if (!isArriving && shooter) {
				if (const auto model = a_this->Get3D()) {
					auto controllers = model->GetControllers();
					if (controllers) {
						auto manager = controllers->AsNiControllerManager();
						if (manager) {
							auto endSequence = manager->GetSequenceByName("mEnd");
							if (endSequence) shooter->PlayAnimation(manager, endSequence, endSequence);
						}
					//	for (auto sequence : manager->activeSequences) {
					//	//	spdlog::debug("active sequence is {}", sequence->name);
					//		auto endSequence = manager->GetSequenceByName("mEnd");
					//		if (endSequence) {
					//			shooter->PlayAnimation(manager, endSequence, endSequence);
					//	//		spdlog::debug("end sequence is {}", endSequence->name);
					//		}
					//	}
					}
				}
			}

			for (auto& point : a_AllCdPointCollector->hits) {
				const auto ourProj 	= RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableA);
				const auto target 	= RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

			//	if (ourProj) {
			//		if (projBase == Leviathan::LeviProjBaseL) a_this->data.angle.z = Config::MaxAxeStuckAngle;
			//		else a_this->data.angle.y = Config::MaxAxeStuckAngle;
			//	}
				if (target && !target->AsProjectile()) {
					if (!Leviathan::isAxeStucked && !isArriving) Leviathan::isAxeStucked = true;
				//	spdlog::debug("levi hitted to {}!!", target->GetName());
					if (target->formType == RE::FormType::ActorCharacter) {
						if (const auto victim = target->As<RE::Actor>()) {
							if (a_this->IsMissileProjectile()) {
								if (Leviathan::leviLastHitActor == victim || Leviathan::leviStuckedActor == victim) isSameTarget = true;
								if (!isArriving) Leviathan::leviStuckedActor = victim;

								if (!target->IsDead()) {
									isTargetActor = true;
									RE::EnchantmentItem* ench = nullptr;
									if (rtData.weaponSource && rtData.weaponSource->formEnchanting) ench = rtData.weaponSource->formEnchanting;

									if (shooter && !isArriving) {
										if (ench) {
											if (const auto eff = ench->GetCostliestEffectItem(); eff && eff->IsHostile()) {
												shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, ench);
											//	ench->data.chargeOverride -= 1000u;
											}
										} else {
											if (rtData.weaponSource)
												shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);
										}
									}
								}
							}
						}
					}
				}
				if ((!isTargetActor || isSameTarget) && isArriving) return true;

	#ifdef EXPERIMENTAL
				const bool isVertical = !isArriving && projBase == Leviathan::LeviProjBaseH;
				auto& angle = a_this->data.angle;
				auto& matrixL = a_this->Get3D2()->local.rotate;
				auto& matrixW = a_this->Get3D2()->world.rotate;
				Leviathan::SetHitRotation(angle, isVertical);
				Leviathan::SetHitRotation(matrixL, isVertical);
				Leviathan::SetHitRotation(matrixW, isVertical);
	#endif
			}

			if (Leviathan::throwState == Leviathan::ThrowState::kThrown) Leviathan::SetThrowState(Leviathan::ThrowState::kCanArrive);
			
			return false;
		}
	/**/
		else if (projBase == Draupnir::DraupnirSpearProjBaseL) {
			for (auto& point : a_AllCdPointCollector->hits) {
				const auto target 	= RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

				if (target) {
					if (target->formType == RE::FormType::ActorCharacter) {
						if (const auto victim = target->As<RE::Actor>()) {
							if (!victim->IsDead()) {
								RE::EnchantmentItem* ench = nullptr;
								if (rtData.weaponSource && rtData.weaponSource->formEnchanting) ench = rtData.weaponSource->formEnchanting;

								if (shooter) {
									if (ench) {
										if (const auto eff = ench->GetCostliestEffectItem(); eff && eff->IsHostile()) {
											shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, ench);
										//	ench->data.chargeOverride -= 1000u;
										}
									} else {
										if (rtData.weaponSource)
											shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);
									}
								}
							}
						}
					}
				}
			}
			return false;
		}
	/**/
#ifdef EXPERIMENTAL
		else if (projBase == Draupnir::DraupnirsCallProjBaseL) {
			for (auto& point : a_AllCdPointCollector->hits) {
				const auto target 	= RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

				if (target) {
					if (target->formType == RE::FormType::ActorCharacter) {
						if (const auto victim = target->As<RE::Actor>()) {
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
#endif
	}
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
		const auto projBase = proj->GetProjectileBase();
		if (projBase == Leviathan::LeviProjBaseA) {
			spdlog::debug("levi impacted to {} while arriving", a2 ? a2->GetName() : "NULL");
			if (a2 && a2->As<RE::Actor>()) Leviathan::leviLastHitActor = a2->As<RE::Actor>();
			proj->GetMissileRuntimeData().impactResult = RE::ImpactResult::kBounce;
			impactData->impactResult = RE::ImpactResult::kBounce;
			Leviathan::Arrive();
		}
		else if (WeaponIdentify::IsRelic(projBase, true)) {
			Leviathan::isAxeStucked = true;
			proj->GetMissileRuntimeData().impactResult = RE::ImpactResult::kStick;
			impactData->impactResult = RE::ImpactResult::kStick;
			Leviathan::leviStuckedBone = impactData->damageRootNode ? impactData->damageRootNode : nullptr;
			spdlog::debug("{} is sticked to {}!", projBase->GetName(), Leviathan::leviStuckedBone ? Leviathan::leviStuckedBone->name : "NULL");
		//	if (projBase == Leviathan::LeviProjBaseL) proj->data.angle.z = Config::MaxAxeStuckAngle;
		//	else proj->data.angle.y = Config::MaxAxeStuckAngle;
			spdlog::debug("hit angle = [{}, {}, {}]", proj->data.angle.x, proj->data.angle.y, proj->data.angle.z);
		}
		else if (WeaponIdentify::IsRelic(projBase, false, true)) {
			proj->GetMissileRuntimeData().impactResult = RE::ImpactResult::kStick;
			impactData->impactResult = RE::ImpactResult::kStick;
/**/
#ifdef EXPERIMENTAL
			const auto hitBone = impactData->damageRootNode;
			spdlog::debug("{} is sticked!", projBase->GetName());
			if (a2 && a2->formType == RE::FormType::ActorCharacter) {
				const auto target = a2->As<RE::Actor>();
				if (target && hitBone) {
					int nthDraupnir = 0;
					for (const auto draupnir : Draupnir::DraupnirSpearProjectiles) {
						if (draupnir && draupnir == proj) {
							const auto& pFlags = proj->GetProjectileRuntimeData().flags;
							if (!(pFlags & (1 << 2)) && !(pFlags & (1 << 22)) && (pFlags & (1 << 25))) spdlog::debug("flag check done!!");
							Draupnir::DraupnirSpearHitBones[nthDraupnir] = hitBone;
							Draupnir::DraupnirSpearHitActors[nthDraupnir] = target;
							spdlog::debug("{}. draupnir attached to {} of {}!",nthDraupnir , hitBone->name, target->GetName());
						} else {
							Draupnir::DraupnirSpearHitBones[nthDraupnir] = nullptr;
							Draupnir::DraupnirSpearHitActors[nthDraupnir] = nullptr;
						}
						nthDraupnir++;
					}
				}
				else spdlog::debug("draupnir attached to null");
			}
#endif
		}
	}
	return impactData;
}
void ProjectileHook::UpdateMovingCone(RE::ConeProjectile* proj, float dtime)
{
	_UpdateMovingCone(proj, dtime);
	{
		float DistanceMoved = proj->GetProjectileRuntimeData().distanceMoved + 1.f;
		const float cone_len = 169.f;
		proj->GetConeRuntimeData().coneAngleTangent = cone_len / DistanceMoved;
	}
}

bool ProjectileHook::GetKillOnCollisionMissile(RE::MissileProjectile* a_this)
{
	return !_GetKillOnCollisionMissile(a_this);
}
bool ProjectileHook::GetKillOnCollisionArrow(RE::ArrowProjectile* a_this)
{
	return !_GetKillOnCollisionArrow(a_this);
}