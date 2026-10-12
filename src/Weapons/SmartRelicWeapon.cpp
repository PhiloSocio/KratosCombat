#include "SmartRelicWeapon.h"
#include "Weapons/States/ArrivingState.h"
#include "Weapons/States/HomingState.h"
#include "Actors/Capabilities/Caller.h"
#include "Actors/Capabilities/Rager.h"
#include "Assets.h"

SmartRelicWeapon::SmartRelicWeapon(RE::TESBoundObject* a_object)
    : ThrowableRelicWeapon(a_object)
{
    abilities.set(RelicAbility::kThrowable, RelicAbility::kCallable, RelicAbility::kSmart);
    Initialize();
}

bool SmartRelicWeapon::Initialize() 
{
    ArrivingWeaponDummyProjectile = CreateBaseProjectile("DummyProjectile", "Dummy Projectile");
    ArrivingWeaponDummyAmmo = CreateBaseAmmo(ArrivingWeaponDummyProjectile, "DummyAmmo", "Dummy Ammo");
    ThrowableWeaponDummyProjectile->data.flags.reset(pbFlag::kCanPickUp);
    return ArrivingWeaponDummyAmmo != nullptr;
}

void SmartRelicWeapon::GetPosition(RE::NiPoint3& a_point)
{
    if (replacedProjectileModel) {
        transformW = GetWorldTransform();
        a_point = transformW.translate * 70.f;
    //    a_point = replacedProjectileModel->worldBound.center;
        spdlog::debug("{} coming from weapon projectileModel location.", weap->GetFullName());
    } else if (auto droppedModel = currentReference ? currentReference->GetCurrent3D() : nullptr) {
        transformW = GetWorldTransform();
        a_point = transformW.translate * 70.f;
        spdlog::debug("{} coming from weapon droppedModel location.", weap->GetFullName());
    } else if (stuckedBone) {
        a_point = replacedProjectileModel ? transformW.translate : stuckedBone->world.translate;
        stuckedBone.reset();

        if (stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
            ObjectUtil::Projectile::DeleteAnExtraArrow(stuckedActor, projectileModel);
#else
            stuckedActor->RemoveExtraArrows3D();
#endif
            spdlog::debug("{}' stucked actor's extra arrows removed", weap->GetFullName());
            stuckedActor.reset();
        } else spdlog::debug("{} not stucked anybody", weap->GetFullName());
    }

    auto callerActor = callerParent ? callerParent->GetActor() : nullptr;
    if (!callerActor) return;

    if (GetWielder()&& GetWielder()->GetActor() && GetThrowState() == ThrowState::kThrowable) {
        if (auto backWeaponSheathe = callerActor->GetNodeByName("WeaponBack"); backWeaponSheathe) {
            const auto& backSheatheTransform = backWeaponSheathe->world;
            a_point = backSheatheTransform.translate;
            const auto& rightDir = backSheatheTransform.rotate * rightVec3;
            const auto& backDir = backSheatheTransform.rotate * backVec3;
            velocity = (0.69f * rightDir + 0.31f * backDir) * 2400.f;
            spdlog::debug("{} is coming from back sheathe of {}", weap->GetFullName(), GetWielder()->GetActor()->GetName());
        }
    }
    auto pcPos = callerActor->GetPosition();
    float dist = pcPos.GetDistance(a_point);
    if (dist > 36000.f) {   // ~42000 is limit
        spdlog::info("{} is too far away from you! ({} m)", weap->GetFullName(), (int)dist / 100);
        auto dir = a_point - pcPos;
        dir.Unitize();
        a_point = pcPos + dir * 36000.f;
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

bool SmartRelicWeapon::Call(Caller* a_caller, const bool a_justDestroy, std::optional<float> a_delay)
{
    caller = a_caller;
    callerParent = caller ? caller->GetParent() : nullptr;
    auto callerActor = callerParent && callerParent->IsValid() ? callerParent->GetActor() : nullptr;
    if (!callerActor) {
        spdlog::debug("caller is invalid");
        return false;
    }

    if (!weap ||
        !ArrivingWeaponDummyAmmo ||
        !ArrivingWeaponDummyProjectile)
    {
        spdlog::debug("called weapon is invalid");
        return false;
    }

    spdlog::debug("{} is calling...", weap->GetFullName());
    trailRemoveUpdate.Done();

    callerActor->SetGraphVariableBool("bLeviInCatchRange", false);

    if (weaponParentNode) {
        transformPW = weaponParentNode->world;
        transformPL = weaponParentNode->local;
    }

    soundData.FadeThrowingLoopSounds(369);

    if (a_justDestroy) {
        if (stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
            ObjectUtil::Projectile::DeleteAnExtraArrow(stuckedActor, projectileModel);
#else
            stuckedActor->RemoveExtraArrows3D();
#endif
            spdlog::debug("{}'s stucked actor's extra arrows removed", weap->GetFullName());
            stuckedActor = nullptr;
        } else {
            spdlog::debug("{} is not stucked anybody", weap->GetFullName());
        }
        return false;
    }
    if (a_delay.has_value() && a_delay.value() > *g_deltaTime) {
        delayedCallUpdate.RegisterForUpdate(a_delay.value(), true);
        return true;
    }

    auto rHandBone = callerParent->GetRHandBone();
    if (!isPenetrating) {
        soundData.PlayCallingHandSounds(rHandBone);
    }

    if (!projectile || projectile->GetProjectileRuntimeData().flags.all(RE::Projectile::Flags::kProcessedImpacts)) {
        if (projectile) {
            if (!isPenetrating) position = projectile->data.location;
            auto& projectileRTD = projectile->GetProjectileRuntimeData();
            auto& pFlags = projectileRTD.flags;
            if (!(pFlags & pFlag::kDestroyed)) {
                pFlags |= pFlag::kDestroyed;
            }
        }

        RE::NiPoint3 startPoint = position;
        GetPosition(startPoint);
        projectileRotation = MathUtil::Algebra::VectorToPitchYaw(direction);
        SetThrowState(ThrowState::kNone);

        RE::Projectile::LaunchData lData(callerActor, startPoint, projectileRotation, ArrivingWeaponDummyAmmo, weap);

        lData.noDamageOutsideCombat = true; //  can be an option
        lData.weaponSource = weap;
        if (weapIED) {
            if (weapIED->IsPoisoned())
                weapIED->PoisonObject(lData.poison, 1u);
            if (weapIED->IsEnchanted() && weapIED->GetEnchantmentCharge() > 0.f)
                lData.enchantItem = weapIED->GetEnchantment();
        }

        RE::ProjectileHandle pHandle;
        if (auto pHandlePtr = RE::Projectile::Launch(&pHandle, lData); pHandlePtr && pHandlePtr->get().get()) {
            projectileHandle = *pHandlePtr;
            launchTime = AsyncUtil::GameTime::GetEngineTime();
            projectile = pHandlePtr->get().get();
        //    currentReference->Disable();

            SetState(RelicWeaponState::Type::kArriving);
            spdlog::info("{} is arriving to {}...", weap->GetFullName(), callerActor->GetDisplayFullName());

            projectileModel = nullptr;
            _modelInitiated = false;
            if (replacedProjectileModel) {
                weaponModelSterilizedCopy = std::move(replacedProjectileModel);
            }
        } else {
            spdlog::warn("{} can't arrive to {}", weap->GetFullName(), callerActor->GetDisplayFullName());
        }
    } else if (projectile) {
        spdlog::debug("the projectile is already active, changing its state to arriving state...");
        SetThrowState(ThrowState::kNone);
        SetState(RelicWeaponState::Type::kArriving);
        spdlog::info("{} is arriving to {}...", weap->GetFullName(), callerActor->GetDisplayFullName());
    }
    return true;
}
void SmartRelicWeapon::Catch(const bool a_justDestroy)
{
    isPenetrating = false;
    if (projectile) {
    //    if (APIs::precision || APIs::Request()) {
    //        APIs::precision->RemoveProjectileCollision(throwerActor->GetHandle(), collisionDefinition);
    //    }

        auto& projectileRTD = projectile->GetProjectileRuntimeData();
        projectileRTD.flags |= pFlag::kDestroyed;
        projectile = nullptr;
        projectileModel = nullptr;
        if (a_justDestroy) return;
    }

    auto callerActor = callerParent ? callerParent->GetActor() : nullptr;
    if (callerActor && !callerParent->GetRightHandRelic()) {
        callerActor->SetGraphVariableBool("bLeviInCatchRange", true);

        SetThrowState(ThrowState::kArrived);

        auto assets = Assets::GetSingleton();
        auto rHandBone = callerParent->GetRHandBone();
        if (auto handEffect = assets->VFXeffects.handFrost; handEffect) 
            callerActor->ApplyArtObject(handEffect, 1.f, nullptr, false, false, rHandBone);

        soundData.FadeArrivingNearSounds(469);
        soundData.FadeArrivingLoopSounds(469);
        soundData.StopArrivingLoopSounds(*g_deltaTimeRealTime * 1200.f);
        soundData.PlayCatchingSounds(rHandBone);

        if (weap && currentReference) {
            callerParent->SetSkipEquipAnim(true);
            callerParent->SetUnequipWhenAnimEnds(false);
            callerParent->DoAction(ActionType::kWeaponCall);
            callerActor->PickUpObject(currentReference, 1, false, false);
        //    currentReference->RemoveItem(weap, 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, callerActor);
            ObjectUtil::Actor::EquipItem(callerActor, weap, callerParent->GetSkipEquipAnim());//, 1U, true, false, false, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, callerActor);
            RE::ShakeCamera(0.3f, position, 0.5f);
            if (callerParent->GetSkipEquipAnim()) callerParent->SetSkipEquipAnim(false);
        } else spdlog::warn("could not find the weapon");

        if (stuckedBone)   stuckedBone    = nullptr;
        if (stuckedActor)  stuckedActor   = nullptr;
        lastHitActors.clear();
        lastHitForms.clear();

        if (Config::UsePrecisionTrails) {
            auto animObjectR = callerParent->GetAnimObjectRBone();
            auto weaponBone = callerParent->GetWeaponBone();
            if (animObjectR && animObjectR->AsNode() && weaponBone && weaponParentNode) {
                animObjectR->AsNode()->AttachChild(weaponParentNode);
                weaponParentNode->local.translate = RE::NiPoint3();
                auto& localRotation = weaponParentNode->local.rotate;
                const auto& desiredParentWorldRotate = weaponBone->world.rotate;
                const auto& parentWorldRotate = animObjectR->world.rotate;
                localRotation = parentWorldRotate.Transpose() * desiredParentWorldRotate;
            }
        }
        trailRemoveUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);
    }
}

bool SmartRelicWeapon::OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) 
{
    if (projectile) {
        auto& rtData = projectile->GetProjectileRuntimeData();
        if (rtData.livingTime <= 1.69f * *g_deltaTime) return true;

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
            if (target && projectile != target->AsProjectile()) {
                if (!isTargetActor && !lastHitForms.empty() && std::find(lastHitForms.begin(), lastHitForms.end(), target) != lastHitForms.end()) isSameTarget = true;
                else if (victim && shooter && victim != shooter) {
                    if (!lastHitActors.empty() && std::find(lastHitActors.begin(), lastHitActors.end(), victim->GetHandle()) != lastHitActors.end()) isSameTarget = true;
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
                        if (!isArriving) stuckedActor.reset(victim);

                        if (weap && ench && ench->effects[0])
                            ObjectUtil::Enchantment::ChargeInventoryWeapon(shooter, weap, -ench->effects[0]->effectItem.magnitude);

                        if (!isArriving && callerParent) {
                            shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);

                            callerParent->RestoreRage(Config::RageBuffRate->value * 0.25f, true);
                        }

                        if (isArriving || isHoming) {
                        //    RE::InventoryEntryData weaponIED;
                        //    weaponIED.object = weap;
                        //    RE::HitData hitData;
                        //    hitData.aggressor = shooter;
                        //    hitData.target = victim;
                        //    hitData.hitPosition = RE::NiPoint3(point.contact.position.quad.m128_f32[0], point.contact.position.quad.m128_f32[1], point.contact.position.quad.m128_f32[2]);
                        //    hitData.hitDirection = direction;
                        //    hitData.weapon = weap;
                        //    hitData.totalDamage = damage;
                        //    hitData.stagger = 1.5f;
                        //    hitData.pushBack = rtData.power;
                        //    hitData.sourceRef = projectile->GetHandle();
                    //        if (weapIED) {
                    //            auto hitData = RE::HitData::Create(shooter, victim, weapIED.get(), false);
                    //            if (hitData) {
                    //                spdlog::warn("{} hit to {} while arriving", hitData->weapon->GetFullName(), hitData->target.get()->GetName());
                    //                hitData->hitPosition = RE::NiPoint3(point.contact.position.quad.m128_f32[0], point.contact.position.quad.m128_f32[1], point.contact.position.quad.m128_f32[2]);
                    //                hitData->hitDirection = direction;
                    //            //    ObjectUtil::Actor::ProcessHit(victim, hitData);
                    //                ObjectUtil::Actor::ProcessHitImpl(shooter, victim, projectile, false);
                    //            }
                    //        }
                        //    RE::TESHavokUtilities::AddExplosionImpulse(projectile->GetCurrent3D(), point.contact.position, rtData.power, hitData);

                            ObjectUtil::Actor::ProcessHitImpl(shooter, victim, projectile, false);
                            return true;
                        }
                    }
                }
            } else {spdlog::warn("WEIRD, target or levi is not exists");}
            if (isSameTarget || (!isTargetActor && (isArriving || isHoming))) {
                return true;
            } else if (!isPenetrating) {
                trailRemoveUpdate.RegisterForUpdate(0.f, false);
                if (projectileModel) {
                    auto controllers = projectileModel->GetControllers();
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
    if (projectile && a_impactData) {
        auto missileProjectile = projectile->As<RE::MissileProjectile>();
        const bool isTargetActor = a_target ? a_target->formType == RE::FormType::ActorCharacter : false;
        const auto victim = isTargetActor ? a_target->As<RE::Actor>() : nullptr;
        auto& rtData = missileProjectile->GetProjectileRuntimeData();
        auto& missileRTD = missileProjectile->GetMissileRuntimeData();
    //    auto shooter = rtData.shooter ? rtData.shooter.get()->As<RE::Actor>() : nullptr;

    //    currentReference->Enable(false);
    //    std::jthread thread([=](){
    //        RE::NiAVObject* droppedWeaponModel = nullptr;
    //        while (droppedWeaponModel == nullptr || droppedWeaponModel->parent == nullptr) droppedWeaponModel = currentReference->GetCurrent3D();
    //        if (droppedWeaponModel && replacedProjectileModel && droppedWeaponModel->parent) {
    //            placeWeaponUpdate.RegisterForUpdate(0.f, true);
    //        }
    //    });
    //    thread.detach();
//        placeWeaponUpdate.RegisterForUpdate(*g_deltaTime, true);

    //    if (favoriteData)
    //        currentReference->extraList.Add(favoriteData);
                //    currentReference->Set3D(replacedProjectileModel.get());
        const bool isHoming = IsHoming();
        const bool isArriving = IsArriving();
        {
            bool skipIt = false;
            bool stopSounds = true;

            if (isTargetActor)  {lastHitActors.insert(lastHitActors.begin(), victim->GetHandle()); if (lastHitActors.size() > 4) lastHitActors.pop_back();}
            else if (a_target)  {lastHitForms.insert(lastHitForms.begin(), a_target); if (lastHitForms.size() > 4) lastHitForms.pop_back();}
            else skipIt = true; //  the target is not an actor or a form, probably the ground or an inpenetrable thing.

            if (isArriving) {
                spdlog::debug("{} impacted to {} while arriving", projectile->GetName(), a_target ? a_target->GetName() : "NULL");
                auto arrivingState = GetCurrentState() ? dynamic_cast<ArrivingState*>(GetCurrentState()) : nullptr;
                if (arrivingState && victim && victim == arrivingState->GetNextTarget())  arrivingState->GetTargets().erase(arrivingState->GetTargets().begin());
            //    for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
            //    missileRTD.impactResult = RE::ImpactResult::kBounce;
            //    a_impactData->impactResult = RE::ImpactResult::kBounce;
            //    projectile = nullptr;
            //    isPenetrating = true;
            //    Call(caller, false);
                stopSounds = false;
            } else if (isHoming) {
                auto homingState = GetCurrentState() ? dynamic_cast<HomingState*>(GetCurrentState()) : nullptr;
                if (isTargetActor && victim == homingState->GetNextTarget()) homingState->GetTargets().erase(homingState->GetTargets().begin());
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kDestroy;
                a_impactData->impactResult = RE::ImpactResult::kDestroy;
                missileRTD.impactResult = RE::ImpactResult::kDestroy;
                projState = ProjectileState::kNone;
                auto pRot = MathUtil::Algebra::VectorToPitchYaw(direction);
                Throw(thrower, RotationType::kSpinLateral, pRot, position);
                stopSounds = false;
                spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projectile->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
            } else if (isPenetrating && skipIt) {
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kNone;
                a_impactData->impactResult = RE::ImpactResult::kNone;
                missileRTD.impactResult = RE::ImpactResult::kNone;
                projState = ProjectileState::kHavok;
                isPenetrating = false;
            } else if (isPenetrating) {
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kNone;
                a_impactData->impactResult = RE::ImpactResult::kNone;
                missileRTD.impactResult = RE::ImpactResult::kNone;
                projState = ProjectileState::kNone;
            //    auto pRot = MathUtil::Algebra::VectorToPitchYaw(direction);
            //    Throw(thrower, RotationType::kSpinLateral, pRot, position);
                stopSounds = false;
                spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projectile->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
            } else if (impactResult == RE::ImpactResult::kStick) {  //  let it trying to stick if it can
                //
            } else if (isTargetActor) { //  don't let it stick to actors but let it do whatever it wants.
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                a_impactData->impactResult = RE::ImpactResult::kBounce;
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                projState = ProjectileState::kHavok;
            }

            missileRTD.impactResult = a_impactData->impactResult;

            const bool itWillStick = a_impactData->impactResult == RE::ImpactResult::kStick;
            const bool itWillBounce = a_impactData->impactResult == RE::ImpactResult::kBounce;
            if (itWillStick) {
        //        if (isTargetActor && rtData.weaponDamage > (victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) / 10.f)/**/) {
        //            ObjectUtil::Actor::PushActorAway(victim, projectile->data.force, rtData.linearVelocity);
        //        }
                stuckedActor.reset(victim);
                stuckedBone.reset(a_impactData->damageRootNode);
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kStick;
                projState = ProjectileState::kStucked;
                spdlog::debug("{} is sticked to {}!", projectile->GetName(), stuckedBone ? stuckedBone->name : "NULL");
            } else if (itWillBounce) {
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kNone;
                projState = ProjectileState::kHavok;
                spdlog::debug("{} is bounced from {}!", projectile->GetName(), a_impactData->damageRootNode ? a_impactData->damageRootNode->name : "NULL");
            }
            if (stopSounds)
                GetSoundManager().FadeThrowingLoopSounds(369);
        }
    }
}
void SmartRelicWeapon::OnTrailDelete()
{
    if (callerParent && callerParent->GetAnimObjectRBone()) {
        callerParent->GetAnimObjectRBone()->AsNode()->DetachChild(replacedProjectileModel->parent);
        if (IsEquipped()) {
            replacedProjectileModel.reset();
        }
    }
}

void SmartRelicWeapon::Update() 
{
    ThrowableRelicWeapon::Update();
    if (delayedCallUpdate.IsTimeToUpdate()) {
        Call(caller, false);
    }
}
void SmartRelicWeapon::UpdateProjectile(RE::Projectile* a_projectile) 
{
    ThrowableRelicWeapon::UpdateProjectile(a_projectile);
    if (currentState) {
        auto status = currentState->Update(*g_deltaTime);
        if (status > RelicWeaponState::Status::kRunning) {
            currentState->Exit();
            currentState.reset();
        }
    }
}
