#include "SmartRelicWeapon.h"
#include "Weapons/States/ArrivingState.h"
#include "Weapons/States/HomingState.h"
#include "Actors/Rager.h"

SmartRelicWeapon::SmartRelicWeapon(RE::TESBoundObject* a_object)
    : ThrowableRelicWeapon(a_object)
{
}


bool SmartRelicWeapon::Initialize() 
{
    return false;
}

void SmartRelicWeapon::Update() 
{}
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
    return true;
}
void PreImpact(RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable)
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
