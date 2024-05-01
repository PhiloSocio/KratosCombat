#include "hook.h"
#include "util.h"
#include "settings.h"
#include "MainKratosCombat.h"

using tState = LeviathanAxe::ThrowState;

static std::mutex ThrowCallMutex;

/* //very good instance for projectile movement controlling from master ersh's true directional movement, 
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
void ProjectileHook::GetLinearVelocityArrow(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
{
	_GetLinearVelocityArrow(a_this, a_outVelocity);
	if (ThrowCallMutex.try_lock()) {
		LeviAndDraupnir(a_this);
		ThrowCallMutex.unlock();
	}
}
void ProjectileHook::LeviAndDraupnir(RE::Projectile* a_this)
{
	auto& runtimeData = a_this->GetProjectileRuntimeData();
	const auto& shooter = runtimeData.shooter;

	if (shooter.native_handle() == 0x100000)	// player only, 0x100000 == player
	{
		auto projectileNode = a_this->Get3D2();
		if (!projectileNode) {spdlog::warn("projectile's 3d not loaded"); return;}
		const auto projBase = a_this->GetProjectileBase();
		const auto& livingTime = runtimeData.livingTime;
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
			
			auto Levi = LeviathanAxe::GetSingleton();
			if (Levi->isAxeStucked) Levi->isAxeStucked = false;
			if (a_this != Levi->LastLeviProjectile)	Levi->LastLeviProjectile = a_this;
			(projBase == Levi->LeviProjBaseL ? Levi->LeviathanAxeProjectileL : Levi->LeviathanAxeProjectileH) = a_this;
			(projBase == Levi->LeviProjBaseL ? Levi->LeviathanAxeProjectileH : Levi->LeviathanAxeProjectileL) = nullptr;

			if (livingTime > 0.4f && Levi->GetThrowState() == tState::kThrown) Levi->SetThrowState(tState::kCanArrive);

			//speed
			auto& vel = runtimeData.linearVelocity;

			auto linearDir = vel;
			linearDir.Unitize();

			if (!Levi->isAxeCalled) {
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
			//	float yAngle = Levi->data.yAngle;		//0.35f = 20 degrees for lateral throw, PI/2 = 90 degrees for vertical throw
				leviAngle.x = asin(linearDir.z);
			//	leviAngle.y = yAngle;
			//	leviAngle.z = rot;
			//	RE::NiMatrix3 copyMatrix = projectileNode->local.rotate;									//	copy of original rotation matrix
			//	MathUtil::Algebra::SetRotationMatrix(copyMatrix, -linearDir.x, linearDir.y, linearDir.z);	//	set rotation of copied matrix
			//	MathUtil::Algebra::RotateMatrixAroundAxisses(copyMatrix, yAngle, 0.f, rot);					//	add rotation
			//	projectileNode->local.rotate = copyMatrix;													//	apply rotation
			//	projectileNode->world.rotate = copyMatrix;

				Levi->data.position = leviPos;
				Levi->data.throwedTime = livingTime;
			}
			else if (Levi->isAxeCalled) {
				if (projBase != Levi->LeviProjBaseA) {
					runtimeData.flags |= (1 << 25); 
				//	spdlog::debug("[HOOK] levi destroyed before call"); 
					return;
				}
				if (Levi->LeviathanAxeProjectileA != a_this) Levi->LeviathanAxeProjectileA = a_this;
				if (Levi->GetThrowState() == tState::kCanArrive) Levi->SetThrowState(tState::kArriving);

				RE::NiPoint3 direction = (handPos - leviPos);						//	direction of the axe to player's hand
				direction.Unitize();												//	normalize direction

			//	float passedArrTime = livingTime - Levi->throwedTime;
			//	if (passedArrTime < 0.f) passedArrTime = livingTime;
				float arrivingTime = Config::ArrivalTime - livingTime;//passedArrTime;
				if (arrivingTime < *g_deltaTime * 2.f) arrivingTime = *g_deltaTime * 2.f;

				const float distance = handPos.GetDistance(leviPos);

			//	speed calculation
				float arrSpeed = distance / arrivingTime;

			//	set speed limits
				const bool isCatchable = distance < Config::CatchingTreshold || distance <= (*g_deltaTime * arrSpeed);
				if (arrSpeed < Config::MinArrivalSpeed && !isCatchable)	arrSpeed = Config::MinArrivalSpeed;
				else if (arrSpeed > Config::MaxArrivalSpeed)			arrSpeed = Config::MaxArrivalSpeed;

				if (isCatchable) {
					if (Levi->GetThrowState() == tState::kArriving) Levi->SetThrowState(tState::kArrived);
					Levi->Catch(a_this, AnArchos);
					spdlog::debug("Levi proj catched");
				}
			//	if (distance <= (*g_deltaTime * arrSpeed)) {
			//		if (distance >= Config::CatchingTreshold) {
			//			if (tState == tState::kArriving) Levi->SetThrowState(tState::kArrived);
//
			//			if (!(runtimeData.flags & (1 << 25))) {
			//				Levi->Catch(a_this, AnArchos);
			//				spdlog::debug("Levi proj catched");
			//			}
			//		}
			//		runtimeData.flags |= (1 << 25);
			//	//	spdlog::debug("levi is catched!! dist {}, speed {}, dt {}, dtRT {}", distance, arrSpeed, *g_deltaTime, *g_deltaTimeRealTime);
			//	}

				const float sinAngle	= Config::ArrivalRoadCurveMagnitude;
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
				const float offset = arrSpeed * sinAngle * curveMult;

				// new direction calculating method
				RE::NiPoint3 crossDir = direction.UnitCross(downVec);
				vel = (direction * arrSpeed) + (crossDir * offset);
			//		spdlog::debug("distance: {}, passed arriving time: {}", distance, passedArrTime);
				RE::NiPoint3 curvyDir = vel;
				curvyDir.Unitize();
				leviAngle.x = asin(curvyDir.z);
				leviAngle.z = atan2(curvyDir.x, curvyDir.y);
			//	MathUtil::Algebra::SetRotationMatrix(projectileNode->local.rotate, -curvyDir.x, curvyDir.y, curvyDir.z);

			//	float xRot	= Config::ArrivalRotationX * livingTime;
			//	float yRot	= Config::ArrivalRotationY * livingTime;
			//	float zRot	= Config::ArrivalRotationZ * livingTime;
			//	float sacb	= curvyDir.x + xRot;
			//	float cacb	= curvyDir.y + yRot;
			//	float sb	= curvyDir.z + zRot;
			//	MathUtil::Algebra::SetRotationMatrix(projectileNode->local.rotate, -sacb, cacb, sb);

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
#ifdef EXPERIMENTAL
		// experimental
		else if (projBase == Draupnir::DraupnirsCallProjBaseL) {
//			const auto lastFormID = a_this->GetFormID();
//			int nextIndex = -1;
//			int nextNextIndex = -1;
//			if (livingTime > 0.3f) goto PROCESS;				//	prevents duplicates
//			for (int i = 0; i <= Config::DraupnirSpearCount; i++) {
//				if (Draupnir::DraupnirsCallProjectiles[i] && Draupnir::DraupnirsCallProjectiles[i]->GetFormID() == lastFormID) return;		//	prevents duplicates
//			}
//
//			for (int i = 0; i <= Config::DraupnirSpearCount; i++) {
//				if (Draupnir::DraupnirsCallProjectiles[i] == nullptr) {
//					nextIndex = i;
//					if (nextIndex == Config::DraupnirSpearCount) nextNextIndex = 0;
//					else nextNextIndex = nextIndex + 1;												//	every spear opens area for next spear
//					break;
//				}
//			}
//			if (nextIndex == -1) return;
//			if (nextNextIndex == -1) {
//				nextNextIndex = 0;
//					spdlog::debug("{}. VERY WEIRD {:08x}", nextIndex, lastFormID);
//			}
//			if (Draupnir::DraupnirsCallProjectiles[nextNextIndex] != nullptr) Draupnir::DraupnirsCallProjectiles[nextNextIndex] = nullptr;
//
//			Draupnir::DraupnirsCallProjectiles[nextIndex] = a_this;
//				spdlog::debug("{}. Draupnir call throwed, ID: {:08x}", (nextIndex + 1), lastFormID);
//
//			PROCESS:

		//	spdlog::debug("draupnir called");
		//	for (const auto proj : Draupnir::DraupnirsCallProjectiles) {
				for (const auto firstBone : Draupnir::DraupnirSpearHitBones) {
					if (firstBone) {
						const auto& targetPos	= firstBone->world.translate;
						const auto& leviPos		= a_this->data.location;
						auto& vel = runtimeData.linearVelocity;
						auto direction = (targetPos - leviPos);
						const auto speed = vel.Length();
						direction.Unitize();
						vel = direction * speed;
					}
				}
		//	}
		//	spdlog::debug("draupnir called 2");
		}
#endif
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

void ProjectileHook::GetCollisionArrow(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (LeviAndDraupnirHit(a_this, a_AllCdPointCollector)) return;
	_GetCollisionArrow(a_this, a_AllCdPointCollector);
}
inline bool ProjectileHook::LeviAndDraupnirHit(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	const auto projBase = a_this->GetProjectileBase();
	if (projBase && WeaponIdentify::IsRelic(projBase)) {
		
		const auto& rtData = a_this->GetProjectileRuntimeData();
		RE::Actor* shooter = nullptr;
		if (rtData.shooter && rtData.shooter.get() && rtData.shooter.get().get())	shooter = rtData.shooter.get().get()->As<RE::Actor>();
	
		if (auto Levi = LeviathanAxe::GetSingleton(); projBase && WeaponIdentify::IsRelic(projBase, true)) {
			Levi->data.model = a_this->Get3D();
			if (Levi->isAxeCalled && Config::DontDamageWhileArrive)	return true;

			const bool isArriving = projBase == Levi->LeviProjBaseA;
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
			//		if (projBase == Levi->LeviProjBaseL) a_this->data.angle.z = Config::MaxAxeStuckAngle;
			//		else a_this->data.angle.y = Config::MaxAxeStuckAngle;
			//	}
				if (target && !target->AsProjectile()) {
					if (!Levi->isAxeStucked && !isArriving) Levi->isAxeStucked = true;
				//	spdlog::debug("levi hitted to {}!!", target->GetName());
					if (target->formType == RE::FormType::ActorCharacter) {
						if (const auto victim = target->As<RE::Actor>()) {
							if (a_this->IsMissileProjectile()) {
								if (Levi->data.lastHitActor == victim || Levi->data.stuckedActor == victim) isSameTarget = true;
								if (!isArriving) Levi->data.stuckedActor = victim;

								if (!target->IsDead()) {
									isTargetActor = true;
									if (Levi->data.weap && Levi->data.ench && Levi->data.ench->effects[0])
										ObjectUtil::Enchantment::ChargeInventoryWeapon(shooter, Levi->data.weap->formID, -Levi->data.ench->effects[0]->effectItem.magnitude);

									if (shooter && !isArriving) {
										shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);
									//	if (ench) {
									//		if (const auto eff = ench->GetCostliestEffectItem(); eff && eff->IsHostile()) {
									//			shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, ench);
									//		//	ench->data.chargeOverride -= 1000u;
									//		}
									//	} else {
									//		if (rtData.weaponSource)
									//			shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);
									//	}
									}
								}
							}
						}
					}
				}
				if ((!isTargetActor || isSameTarget) && isArriving) return true;

#ifdef EXPERIMENTAL
			//	const bool isVertical = !isArriving && projBase == Levi->LeviProjBaseH;
			//	auto offset = projBase->data.collisionRadius;
			//	auto& pos = a_this->data.location;
			//	auto& angle = a_this->data.angle;
			//	auto& matrixL = a_this->Get3D2()->local.rotate;
			//	auto& matrixW = a_this->Get3D2()->world.rotate;
			//	auto hitDir = rtData.linearVelocity;
			//	hitDir.Unitize();
			//	Levi->SetHitRotation(angle, hitDir, isVertical);
			//	Levi->TweakHitPosition(pos, hitDir, offset, isVertical);
			//	Levi->SetHitRotation(matrixL, isVertical);
			//	Levi->SetHitRotation(matrixW, isVertical);
#endif
			}

			if (Levi->GetThrowState() == tState::kThrown) Levi->SetThrowState(tState::kCanArrive);
			
			return false;
		}
	/**/
		else if (projBase == Draupnir::DraupnirSpearProjBaseL) {
			for (auto& point : a_AllCdPointCollector->hits) {
				const auto target = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

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
							for (int i = 0; i <= 9; i++) {
								if (victim == Draupnir::DraupnirSpearHitActors[i]) {
									victim->RemoveExtraArrows3D();
									spdlog::debug("draupnir's call collided with", victim->GetName());
								//	spdlog::debug("{}'s {} is removed from list", target->GetName(), Draupnir::DraupnirSpearHitBones[i]->name);
								//	Draupnir::DraupnirSpearHitBones[i] = nullptr;
								//	Draupnir::DraupnirSpearHitActors[i] = nullptr;
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
/**/
RE::Projectile::ImpactData* ProjectileHook::GetArrowImpactData(RE::ArrowProjectile *proj, RE::TESObjectREFR *a2, RE::NiPoint3 *a3, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, char a7)
{
	auto impactData = _GetArrowImpactData(proj, a2, a3, a_velocity, a_collidable, a6, a7);
	if (proj && impactData) {
		const auto projBase = proj->GetProjectileBase();
		auto Levi = LeviathanAxe::GetSingleton();
		if (projBase == Levi->LeviProjBaseA) {
			spdlog::debug("levi impacted to {} while arriving", a2 ? a2->GetName() : "NULL");
			if (a2 && a2->As<RE::Actor>()) Levi->data.lastHitActor = a2->As<RE::Actor>();
			proj->GetMissileRuntimeData().impactResult = RE::ImpactResult::kBounce;
			impactData->impactResult = RE::ImpactResult::kBounce;
			Levi->Call();
		}
		else if (WeaponIdentify::IsRelic(projBase, true)) {
			Levi->isAxeStucked = true;
			proj->GetMissileRuntimeData().impactResult = RE::ImpactResult::kStick;
			impactData->impactResult = RE::ImpactResult::kStick;
			Levi->data.stuckedBone = impactData->damageRootNode ? impactData->damageRootNode : nullptr;
			spdlog::debug("{} is sticked to {}!", projBase->GetName(), Levi->data.stuckedBone ? Levi->data.stuckedBone->name : "NULL");

		//	const bool isVertical = projBase == Levi->LeviProjBaseH;
		//	auto offset = projBase->data.collisionRadius;
		//	auto& pos = proj->data.location;
		//	auto& angle = proj->data.angle;
		//	auto& matrixL = proj->Get3D2()->local.rotate;
		//	auto& matrixW = proj->Get3D2()->world.rotate;
		//	auto& rtData = proj->GetProjectileRuntimeData();
		//	auto hitDir = rtData.linearVelocity;
		//	hitDir.Unitize();
		//	Levi->SetHitRotation(angle, hitDir, isVertical);
		//	Levi->TweakHitPosition(pos, hitDir, offset, isVertical);
		//	Levi->SetHitRotation(matrixL, isVertical);
		//	Levi->SetHitRotation(matrixW, isVertical);
			spdlog::debug("hit angle = [{}, {}, {}]", proj->data.angle.x, proj->data.angle.y, proj->data.angle.z);
		}
		else if (WeaponIdentify::IsRelic(projBase, false, true)) {
			proj->GetMissileRuntimeData().impactResult = RE::ImpactResult::kStick;
			impactData->impactResult = RE::ImpactResult::kStick;
/**/
#ifdef EXPERIMENTAL
			const auto hitBone = impactData->damageRootNode;
			if (a2 && a2->formType == RE::FormType::ActorCharacter) {
				const auto target = a2->As<RE::Actor>();
				if (target && hitBone) {
					int nthDraupnir = 0;
					for (int i = 0; i <= 9; i++) {
						if (Draupnir::DraupnirSpearProjectiles[i] && Draupnir::DraupnirSpearProjectiles[i] == proj) {
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
#ifdef EXPERIMENTAL
		else if (projBase == Draupnir::DraupnirsCallProjBaseL) {
			if (const auto target = a2->As<RE::Actor>()) {
			//	target->RemoveExtraArrows3D();
				for (int i = 0; i <= 9; i++) {
					if (target == Draupnir::DraupnirSpearHitActors[i]) {
						spdlog::debug("{}'s {} is removed from list", target->GetName(), Draupnir::DraupnirSpearHitBones[i]->name);
						Draupnir::DraupnirSpearHitBones[i] = nullptr;
						Draupnir::DraupnirSpearHitActors[i] = nullptr;
					}
				}
			}
			proj->GetMissileRuntimeData().impactResult = RE::ImpactResult::kImpale;
			impactData->impactResult =  RE::ImpactResult::kImpale;
			spdlog::debug("draupnir's call exploded for an actor!!");
		}
#endif
	}
	return impactData;
}
bool ProjectileHook::GetKillOnCollisionArrow(RE::ArrowProjectile* a_this)
{
	return !_GetKillOnCollisionArrow(a_this);
}

void PlayerHook::OnEquipItem(RE::PlayerCharacter* a_this, bool a_playAnim)
{
	_OnEquipItem(a_this, !SkipAnim(a_this, a_playAnim));
}

bool PlayerHook::SkipAnim(RE::PlayerCharacter* a_this, bool a_playAnim)
{
	if (a_playAnim && a_this) {
		if (auto kratos = Kratos::GetSingleton(); auto levi = LeviathanAxe::GetSingleton()) {
			if (WeaponIdentify::skipEquipAnim) {
				auto obj = a_this->GetEquippedObject(false);
				if (!obj) {
					WeaponIdentify::isLeviathanAxe = false;
					WeaponIdentify::isRelic = false;
						return true;
				}

				auto weap = obj ? obj->As<RE::TESObjectWEAP>() : nullptr;
				if (weap) {
				//	weap->InitItem();
				//	weap->LoadGraphics(weap->AsReference());
				//	if (weap->HasWorldModel()) spdlog::debug("it has world model!");
				}

				WeaponIdentify::skipEquipAnim = false;
			}
		}
	} return false;
}