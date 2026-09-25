#include "SmartRelicWeapon.h"
#include "Weapons/States/ArrivingState.h"
#include "Weapons/States/HomingState.h"
#include "Actors/Rager.h"
#include "Assets.h"

SmartRelicWeapon::SmartRelicWeapon(RE::TESBoundObject* a_object)
    : ThrowableRelicWeapon(a_object)
{
}


bool SmartRelicWeapon::Initialize() 
{
    return false;
}

void SmartRelicWeapon::GetPosition(RE::NiPoint3& a_point)
{
    if (runtimeData.replacedProjectileModel) {
        runtimeData.transformW = GetWorldTransform();
        a_point = runtimeData.transformW.translate * 70.f;
    //    a_point = runtimeData.replacedProjectileModel->worldBound.center;
        spdlog::debug("leviathan coming from weapon projectileModel location.");
    } else spdlog::debug("we can't get leviathan's weapon projectileModel!");

    if (runtimeData.stuckedBone) {
        a_point = runtimeData.replacedProjectileModel ? runtimeData.transformW.translate : runtimeData.stuckedBone->world.translate;
        runtimeData.stuckedBone.reset();

        if (runtimeData.stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
            ObjectUtil::Projectile::DeleteAnExtraArrow(runtimeData.stuckedActor, runtimeData.projectileModel);
#else
            runtimeData.stuckedActor->RemoveExtraArrows3D();
#endif
            spdlog::debug("levi stucked actor's extra arrows removed");
            runtimeData.stuckedActor.reset();
        } else spdlog::debug("levi not stucked anybody");
    } else spdlog::debug("levi not stucked any bone");

    auto throwerActor = runtimeData.thrower ? runtimeData.thrower->GetActor() : nullptr;
    if (!throwerActor) return;

    if (GetThrowState() == ThrowState::kThrowable) {
        if (auto backWeaponSheathe = throwerActor->GetNodeByName("WeaponBack"); backWeaponSheathe) {
            const auto& backSheatheTransform = backWeaponSheathe->world;
            a_point = backSheatheTransform.translate;
            const auto& rightDir = backSheatheTransform.rotate * rightVec3;
            const auto& backDir = backSheatheTransform.rotate * backVec3;
            runtimeData.velocity = (0.69f * rightDir + 0.31f * backDir) * 2400.f;
            spdlog::debug("levi is coming from your back sheathe");
        }
    } else {
        auto pcPos = throwerActor->GetPosition();
        float dist = pcPos.GetDistance(a_point);
        if (dist > 36000.f) {   // ~42000 is limit
            spdlog::info("levi is too far away from you! ({} m)", (int)dist / 100);
            auto dir = a_point - pcPos;
            dir.Unitize();
            a_point = pcPos + dir * 36000.f;
        }
    }
}

void SmartRelicWeapon::SetState(std::unique_ptr<RelicWeaponState> a_state)
{
    if (currentState) {
        currentState->Exit();
    }

    currentState = std::move(a_state);

    if (currentState) {
        currentState->Enter();
    }
}

void SmartRelicWeapon::Call(const bool a_justDestroy, std::optional<float> a_delay)
{
    if (runtimeData.caller && runtimeData.caller->IsValid() && weap) {
        spdlog::debug("Levi is calling...");
        projectileUpdate.Done();

        trailUpdate.Done();
        trailRemoveUpdate.Done();

        if (runtimeData.projectileModel) {
            runtimeData.transformPW = runtimeData.projectileModel->world;
            runtimeData.transformPL = runtimeData.projectileModel->local;
        }
        runtimeData.projectileModel = nullptr;
    //    if (runtimeData.replacedProjectileModel) {
    //        if (runtimeData.projState == ProjectileState::kHavok && runtimeData.replacedProjectileModel->collisionObject && runtimeData.replacedProjectileModel->collisionObject->AsBhkRigidBody()) {
    //            RE::hkTransform rbTransform;
    //            runtimeData.replacedProjectileModel->collisionObject->AsBhkRigidBody()->GetTransform(rbTransform);
    //            RE::NiTransform niTransform;
    //            niTransform.translate = MathUtil::Algebra::HkVectorToNiPoint(rbTransform.translation);
    //            niTransform.rotate = MathUtil::Algebra::HKMatrixToNiMatrix(rbTransform.rotation);
    //            runtimeData.transformW = niTransform;
    //            runtimeData.transformL = niTransform;
    //        } else {
    //            runtimeData.transformW = runtimeData.replacedProjectileModel->world;
    //            runtimeData.transformL = runtimeData.replacedProjectileModel->local;
    //        }
    //    }

        soundData.FadeThrowingLoopSounds(369);

        if (runtimeData.projectile) {
            if (!runtimeData.isPenetrating) runtimeData.position = runtimeData.projectile->data.location;
            auto& projectileRTD = runtimeData.projectile->GetProjectileRuntimeData();
            auto& pFlags = projectileRTD.flags;
            if (!(pFlags & pFlag::kDestroyed)) {
                pFlags |= pFlag::kDestroyed;
            } else  spdlog::debug("levi is already destroyed");

            if (a_justDestroy) {
                if (runtimeData.stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
                    ObjectUtil::Projectile::DeleteAnExtraArrow(runtimeData.stuckedActor, runtimeData.projectileModel);
#else
                    runtimeData.stuckedActor->RemoveExtraArrows3D();
#endif
                    spdlog::debug("levi stucked actor's extra arrows removed");
                    runtimeData.stuckedActor = nullptr;
                } else spdlog::debug("levi not stucked anybody");
                    return;
            }
        } else {spdlog::debug("Stucked Levi is nullptr!");}

        if (auto AnArchos = runtimeData.caller->GetActor(); !a_justDestroy && AnArchos && ArrivingWeaponDummyAmmo && ArrivingWeaponDummyProjectile) {
            RE::NiPoint3 startPoint = runtimeData.position;
            auto rHandBone = runtimeData.caller->GetRHandBone();
            RE::NiPoint3 targetPoint = rHandBone ? rHandBone->world.translate : AnArchos->GetPosition();
            if (!runtimeData.isPenetrating) {
                soundData.PlayCallingHandSounds(rHandBone);
                GetPosition(startPoint);
            }
            RE::ProjectileHandle pHandle;
            runtimeData.projectileRotation = MathUtil::Algebra::VectorToPitchYaw(runtimeData.direction);
            Throw(RotationType::kNone, runtimeData.projectileRotation, startPoint);
/*
            RE::Projectile::LaunchData lData(AnArchos, startPoint, runtimeData.projectileRotation, ArrivingWeaponDummyAmmo, weap);

            lData.noDamageOutsideCombat = true; //  can be an option
            lData.weaponSource = weap;
#ifdef EXPERIMENTAL_THROWPOISON
            lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(AnArchos, false);
#endif
            if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
                lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

            runtimeData.projectileHandle = RE::Projectile::Launch(&pHandle, lData);
            runtimeData.projectile = pHandle.get().get();

            projectileUpdate.RegisterForUpdate(0.0f, false);
*/
            SetState(RelicWeaponState::Type::kArriving);
            SetThrowState(ThrowState::kNone);
            spdlog::info("Levi is arriving...");
        } else {spdlog::warn("WEIRD SpellLeviProjA is nullptr!");}
    } else {spdlog::warn("WEIRD you don't have the axe for calling!!");}
}
void SmartRelicWeapon::Catch(const bool a_justDestroy)
{
    runtimeData.isPenetrating = false;
    if (runtimeData.projectile) {
    //    if (APIs::precision || APIs::Request()) {
    //        APIs::precision->RemoveProjectileCollision(throwerActor->GetHandle(), collisionDefinition);
    //    }

        auto& projectileRTD = runtimeData.projectile->GetProjectileRuntimeData();
        projectileRTD.flags |= pFlag::kDestroyed;
        if (a_justDestroy) return;
    }

    auto callerActor = runtimeData.caller ? runtimeData.caller->GetActor() : nullptr;
    if (callerActor && !runtimeData.caller->GetRightHandRelic()) {
        callerActor->SetGraphVariableBool("bLeviInCatchRange", true);

        SetThrowState(ThrowState::kArrived);

        auto assets = Assets::GetSingleton();
        auto rHandBone = runtimeData.caller->GetRHandBone();
        if (auto handEffect = assets->VFXeffects.handFrost; handEffect) 
            callerActor->ApplyArtObject(handEffect, 1.f, nullptr, false, false, rHandBone);

        soundData.FadeArrivingNearSounds(469);
    //    soundData.FadeArrivingLoopSounds(469);
        soundData.StopArrivingLoopSounds(*g_deltaTimeRealTime * 1200.f);
        soundData.PlayCatchingSounds(rHandBone);

        if (weap) {
            runtimeData.caller->SetSkipEquipAnim(true);
            runtimeData.caller->SetUnequipWhenAnimEnds(false);
            if (callerActor->IsPlayerRef())
                Config::SpecialWeapon->value = (uint8_t)RelicType::kNone;
            callerActor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            runtimeData.caller->DoAction(ActionType::kWeaponCharge);
            ObjectUtil::Actor::EquipItem(callerActor, weap, runtimeData.caller->GetSkipEquipAnim());//, 1U, true, false, false, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, callerActor);
            RE::ShakeCamera(0.3f, runtimeData.position, 0.5f);
            if (runtimeData.caller->GetSkipEquipAnim()) runtimeData.caller->SetSkipEquipAnim(false);
        } else spdlog::warn("you not have the leviathan axe");

        if (runtimeData.stuckedBone)   runtimeData.stuckedBone    = nullptr;
        if (runtimeData.stuckedActor)  runtimeData.stuckedActor   = nullptr;
        runtimeData.lastHitActors.clear();
        runtimeData.lastHitForms.clear();

        if (Config::UsePrecisionTrails) {
            if (runtimeData.caller->GetAnimObjectRBone() && runtimeData.caller->GetAnimObjectRBone()->AsNode() && runtimeData.replacedProjectileModel && runtimeData.replacedProjectileModel->parent) {
                runtimeData.caller->GetAnimObjectRBone()->AsNode()->AttachChild(runtimeData.replacedProjectileModel->parent);
                runtimeData.replacedProjectileModel->parent->local.translate = RE::NiPoint3();
                runtimeData.replacedProjectileModel->parent->local.rotate = runtimeData.replacedProjectileModel->parent->local.rotate * RE::NiMatrix3(PI2, 0.f, PI2);
            }
        }
        trailUpdate.Done();
        runtimeData.projectileModel = nullptr;
        trailRemoveUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);
    }
}

bool SmartRelicWeapon::OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) 
{
    const auto projBase = ThrowableWeaponDummyProjectile;
    if (projBase && runtimeData.projectile) {
        auto& rtData = runtimeData.projectile->GetProjectileRuntimeData();
        RE::Actor* shooter = nullptr;
        if (auto shooterRef = rtData.shooter.get()) shooter = shooterRef->As<RE::Actor>();

        const bool isHoming = IsHoming();
        const bool isArriving = IsArriving();
        if (/*rtData.livingTime < 0.01f || */(isArriving && Config::DontDamageWhileArrive)) return true;
        for (auto& point : a_AllCdPointCollector->hits) {
        //    const auto ourProj  = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableA);
            const auto target   = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

            bool isSameTarget = false;
            bool isTargetActor = target ? target->formType == RE::FormType::ActorCharacter : false;
            const auto victim = target ? target->As<RE::Actor>() : nullptr;
            if (target && runtimeData.projectile != target->AsProjectile()) {
                if (!isTargetActor && !runtimeData.lastHitForms.empty() && std::find(runtimeData.lastHitForms.begin(), runtimeData.lastHitForms.end(), target) != runtimeData.lastHitForms.end()) isSameTarget = true;
                else if (victim && victim != shooter) {
                    if (!runtimeData.lastHitActors.empty() && std::find(runtimeData.lastHitActors.begin(), runtimeData.lastHitActors.end(), victim->GetHandle()) != runtimeData.lastHitActors.end()) isSameTarget = true;
                    if (isHoming) {
                        auto homingState = GetCurrentState() ? dynamic_cast<HomingState*>(GetCurrentState()) : nullptr;
                        if (homingState && victim == homingState->GetNextTarget() && isSameTarget && rtData.livingTime > 0.5f) {
                            isSameTarget = false;
                        }
                    }
                    if (isArriving) {
                        auto arrivingState = GetCurrentState() ? dynamic_cast<ArrivingState*>(GetCurrentState()) : nullptr;
                        if (arrivingState && victim == arrivingState->GetNextTarget() && isSameTarget && rtData.livingTime > 0.5f) {
                            isSameTarget = false;
                        }
                    }
                    if (!victim->IsDead()) {
                        if (!isArriving) runtimeData.stuckedActor.reset(victim);

                        if (weap && ench && ench->effects[0])
                            ObjectUtil::Enchantment::ChargeInventoryWeapon(shooter, weap, -ench->effects[0]->effectItem.magnitude);

                        if (!isArriving) {
                            shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);

                            auto rager = GetThrower() ? dynamic_cast<Rager*>(GetThrower()) : nullptr;
                            rager->RestoreRage(rager->CalcRageDamageOrBuffAmount(rager->GetBuffAmount(), 0.25f), true);
                        }
                    }
                }
            } else {spdlog::warn("WEIRD, target or levi is not exists");}
            if (isSameTarget || (!isTargetActor && (isArriving || isHoming))) {
                return true;
            } else {
                trailUpdate.Done();
                trailRemoveUpdate.RegisterForUpdate(0.f, false);
                if (runtimeData.projectileModel) {
                    auto controllers = runtimeData.projectileModel->GetControllers();
                    if (controllers) {
                        auto manager = controllers->AsNiControllerManager();
                        if (manager) {
                            auto endSequence = manager->GetSequenceByName("mEnd");
                            if (endSequence && shooter) shooter->PlayAnimation(manager, endSequence, endSequence);
                        }
                    }
                }
            }
        }

        if (GetThrowState() == ThrowState::kThrown) SetThrowState(ThrowState::kCanArrive);
        
    }
    return false;
}
void SmartRelicWeapon::PreImpact(RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable)
{}
void SmartRelicWeapon::PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) 
{
    if (runtimeData.projectile && a_impactData) {
        auto missileProjectile = runtimeData.projectile->As<RE::MissileProjectile>();
        const bool isTargetActor = a_target ? a_target->formType == RE::FormType::ActorCharacter : false;
        const auto victim = isTargetActor ? a_target->As<RE::Actor>() : nullptr;
        auto& rtData = missileProjectile->GetProjectileRuntimeData();
        auto& missileRTD = missileProjectile->GetMissileRuntimeData();
        auto shooter = rtData.shooter ? rtData.shooter.get()->As<RE::Actor>() : nullptr;
        const auto projBase = missileProjectile->GetProjectileBase();

        trailUpdate.Done();

        if (!runtimeData.isPenetrating) {
            bool skipIt = false;
            bool stopSounds = true;
            const bool isHoming = IsHoming();
            const bool isArriving = IsArriving();

            if (isTargetActor)  {runtimeData.lastHitActors.insert(runtimeData.lastHitActors.begin(), victim->GetHandle()); if (runtimeData.lastHitActors.size() > 4) runtimeData.lastHitActors.pop_back();}
            else if (a_target)  {runtimeData.lastHitForms.insert(runtimeData.lastHitForms.begin(), a_target); if (runtimeData.lastHitForms.size() > 4) runtimeData.lastHitForms.pop_back();}
            else skipIt = true; //  the target is not an actor or a form, probably the ground or an inpenetrable thing.

            if (isArriving) {
                spdlog::debug("levi impacted to {} while arriving", a_target ? a_target->GetName() : "NULL");
                auto arrivingState = GetCurrentState() ? dynamic_cast<ArrivingState*>(GetCurrentState()) : nullptr;
                if (arrivingState && victim && victim == arrivingState->GetNextTarget())  arrivingState->GetTargets().erase(arrivingState->GetTargets().begin());
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                a_impactData->impactResult = RE::ImpactResult::kBounce;
                runtimeData.isPenetrating = true;
                Call(false);
                stopSounds = false;
            } else if (isHoming) {
                auto homingState = GetCurrentState() ? dynamic_cast<HomingState*>(GetCurrentState()) : nullptr;
                if (isTargetActor && victim == homingState->GetNextTarget()) homingState->GetTargets().erase(homingState->GetTargets().begin());
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kDestroy;
                a_impactData->impactResult = RE::ImpactResult::kDestroy;
                missileRTD.impactResult = RE::ImpactResult::kDestroy;
                runtimeData.projState = ProjectileState::kNone;
                auto pRot = MathUtil::Algebra::VectorToPitchYaw(runtimeData.direction);
                Throw(RotationType::kSpinLateral, pRot, runtimeData.position);
                stopSounds = false;
                spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
            } else if (runtimeData.isPenetrating && skipIt) {
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                a_impactData->impactResult = RE::ImpactResult::kBounce;
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                runtimeData.projState = ProjectileState::kHavok;
            } else if (runtimeData.isPenetrating) {
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kDestroy;
                a_impactData->impactResult = RE::ImpactResult::kDestroy;
                missileRTD.impactResult = RE::ImpactResult::kDestroy;
                runtimeData.projState = ProjectileState::kNone;
                auto pRot = MathUtil::Algebra::VectorToPitchYaw(runtimeData.direction);
                Throw(RotationType::kSpinLateral, pRot, runtimeData.position);
                stopSounds = false;
                spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
            } else if (runtimeData.impactResult == RE::ImpactResult::kStick) {  //  let it trying to stick if it can
                //
            } else if (isTargetActor) { //  don't let it stick to actors but let it do whatever it wants.
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                a_impactData->impactResult = RE::ImpactResult::kBounce;
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                runtimeData.projState = ProjectileState::kHavok;
            }

            missileRTD.impactResult = a_impactData->impactResult;

            const bool itWillStick = a_impactData->impactResult == RE::ImpactResult::kStick;
            const bool itWillBounce = a_impactData->impactResult == RE::ImpactResult::kBounce;
            if (itWillStick) {
        //        if (isTargetActor && rtData.weaponDamage > (victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) / 10.f)/**/) {
        //            ObjectUtil::Actor::PushActorAway(victim, projBase->data.force, rtData.linearVelocity);
        //        }
                runtimeData.stuckedActor.reset(victim);
                runtimeData.stuckedBone.reset(a_impactData->damageRootNode);
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kStick;
                runtimeData.projState = ProjectileState::kStucked;
                spdlog::debug("{} is sticked to {}!", projBase->GetName(), runtimeData.stuckedBone ? runtimeData.stuckedBone->name : "NULL");
            } else if (itWillBounce) {
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                runtimeData.projState = ProjectileState::kHavok;
                spdlog::debug("{} is bounced from {}!", projBase->GetName(), runtimeData.stuckedBone ? runtimeData.stuckedBone->name : "NULL");
            }
            if (stopSounds)
                GetSoundManager().FadeThrowingLoopSounds(369);
        }
    }
}
void SmartRelicWeapon::OnTrailDelete()
{
    if (runtimeData.caller->GetAnimObjectRBone()) {
        runtimeData.caller->GetAnimObjectRBone()->AsNode()->DetachChild(runtimeData.replacedProjectileModel->parent);
    }
}

void SmartRelicWeapon::Update() 
{}
