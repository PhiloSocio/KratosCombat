#include "SmartRelicWeapon.h"
#include "Weapons/States/ArrivingState.h"
#include "Weapons/States/HomingState.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Thrower.h"
#include "Actors/Capabilities/Caller.h"
#include "Actors/Capabilities/Rager.h"
#include "Assets.h"
#include "DebugUtil.h"

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

ActorCapability* SmartRelicWeapon::GetActiveController() const noexcept
{
    if (IsArriving() && GetCaller()) return GetCaller();
    else if (IsHoming() && GetThrower()) return GetThrower();
    else return nullptr;
}
void SmartRelicWeapon::GetPosition(RE::NiPoint3& a_point)
{
    auto callerActor = callerParent ? callerParent->GetActor() : nullptr;
    if (!callerActor) return;

    if (replacedProjectileModel) {
        transformW = GetWorldTransform();
        a_point = transformW.translate * 70.f;
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
            stuckedActor.get()->RemoveExtraArrows3D();
#endif
            spdlog::debug("{}' stucked actor's extra arrows removed", weap->GetFullName());
            stuckedActor.reset();
        } else spdlog::debug("{} not stucked anybody", weap->GetFullName());
    } else if (const auto carrier = GetCarrier(); const auto carrierActor = carrier ? carrier->GetActor() : nullptr) {
        if (auto backWeaponSheathe = carrierActor->GetNodeByName("WeaponBack"); backWeaponSheathe) {
            const auto& backSheatheTransform = backWeaponSheathe->world;
            a_point = backSheatheTransform.translate;
            const auto& rightDir = backSheatheTransform.rotate * rightVec3;
            const auto& backDir = backSheatheTransform.rotate * backVec3;
            velocity = (0.69f * rightDir + 0.31f * backDir) * 2400.f;
            spdlog::debug("{} is coming from back sheathe of {}", weap->GetFullName(), carrierActor->GetName());
        }
    }

    const auto callerPos = callerActor->GetPosition();
    const float dist = callerPos.GetDistance(a_point);
    if (dist > 36000.f) {   // ~42000 is limit
        spdlog::info("{} is too far away from you! ({} m)", weap->GetFullName(), (int)dist / 100);
        auto dir = a_point - callerPos;
        dir.Unitize();
        a_point = callerPos + dir * 36000.f;
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
        if (projState != ProjectileState::kTransition)
            projState = ProjectileState::kHoming;
    } else {    //  reset
        if (projState == ProjectileState::kHoming ||
            projState == ProjectileState::kTransition)
        {
            projState = ProjectileState::kNone;
        }
        SetThrowState(ThrowState::kNone);
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
            stuckedActor.get()->RemoveExtraArrows3D();
#endif
            spdlog::debug("{}'s stucked actor's extra arrows removed", weap->GetFullName());
            stuckedActor.reset();
        } else {
            spdlog::debug("{} is not stucked anybody", weap->GetFullName());
        }
        return false;
    }
    if (a_delay.has_value() && a_delay.value() > *g_deltaTime) {
        delayedCallUpdate.RegisterForUpdate(a_delay.value(), true);
        return true;
    }

    auto& rHandBone = callerParent->GetRHandBone();
    if (!isPenetrating) {
        soundData.PlayCallingHandSounds(reinterpret_cast<RE::NiAVObject**>(&rHandBone));
    }

    bool isProjectileDestroyed = false, isParentsMatching = false, isProjectileActive = false, isProjectileMoving = false;
    projectile = projectileHandle.get().get();
    if (projectile) {
        isParentsMatching     = projectile->GetParentCell() == callerActor->GetParentCell();
        isProjectileDestroyed = projectile->GetProjectileRuntimeData().flags.all(RE::Projectile::Flags::kDestroyed);
        isProjectileActive    = !projectile->GetProjectileRuntimeData().flags.all(RE::Projectile::Flags::kProcessedImpacts);
        isProjectileMoving    = isParentsMatching && isProjectileActive;
    }

    if (!isProjectileMoving) {
        if (projectile) {
            if (projState != ProjectileState::kTransition && projState == ProjectileState::kStucked && projectileModel) {
                constexpr float wobbleDuration = 0.18f;
                Projectile::Manager::GetSingleton()->wobble.emplace_back(projectileModel->world.rotate, projectileHandle, 0.f);
                delayedCallUpdate.RegisterForUpdate(wobbleDuration, true);
                projState = ProjectileState::kTransition;
                SetState(RelicWeaponState::Type::kArriving);
                spdlog::debug("{} is woobling to leave where it is stucked", weap->GetFullName());
                return false;
            } else {
                projectile->Kill();
                projectileHandle.reset();
                projectileModel.reset();
                projectile = nullptr;
            }
        }

        RE::NiPoint3 startPoint = position;
        GetPosition(startPoint);
        direction = rHandBone->world.translate - startPoint;
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
            projectile->inGameFormFlags.set(RE::TESForm::InGameFormFlag::kForcedPersistent);
        //    currentReference->Disable();

            if (IsArriving()) {
                if (projState == ProjectileState::kTransition) {
                    projState = ProjectileState::kNone;
                }
            } else {
                SetState(RelicWeaponState::Type::kArriving);
            }
            spdlog::info("{} is arriving to {}...", weap->GetFullName(), callerActor->GetDisplayFullName());

            projectileModel.reset();
            _modelInitiated = false;
            if (replacedProjectileModel) {
                weaponModelSterilizedCopy = std::move(replacedProjectileModel);
                weaponModelSterilizedCopy->SetMotionType(RE::hkpMotion::MotionType::kKeyframed, true, true, true);
                if (weaponModelSterilizedCopy->parent) {
                    weaponModelSterilizedCopy->parent->DetachChild(weaponModelSterilizedCopy.get());
                    weaponModelSterilizedCopy->parent = nullptr;
                }
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

    lastHitActors.clear();
    lastHitForms.clear();

    targetLocalTranslation = RE::NiPoint3();
    _translationBlended = false;
    _hitTransformApplied = false;
    thrower = nullptr;
    throwerParent = nullptr;
    return true;
}
void SmartRelicWeapon::Catch(const bool a_justDestroy)
{
    isPenetrating = false;
    projectile = projectileHandle.get().get();  //  probably unnecessary but just in case.
    auto callerActor = (callerParent && callerParent->IsValid()) ? callerParent->GetActor() : nullptr;
    if (projectile) {
        if (!a_justDestroy && (!callerActor || callerParent->GetRightHandRelic())) {
            SetState(nullptr);
            caller = nullptr;
            return;
        }

        projectile->Kill();
        projectileHandle.reset();
        projectileModel.reset();
        projectile = nullptr;
        if (a_justDestroy) return;
    }

    if (callerActor) {
        callerActor->SetGraphVariableBool("bLeviInCatchRange", true);

        soundData.FadeArrivingNearSounds(469);
        soundData.FadeArrivingLoopSounds(469);
        soundData.StopArrivingLoopSounds(*g_deltaTime * 3200.f);

        bool success = false;
        if (weap && droppedWeaponKeep.get().get()) {
        //    callerParent->DoAction(ActionType::kWeaponCall);
    //        callerActor->PickUpObject(currentReference, 1, false, false);
            droppedWeaponKeep.get().get()->RemoveItem(weap, 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, callerActor);
            spdlog::debug("{} catched", weap->GetFullName());
            success = true;
        } else if (weap && callerParent == GetCarrier()) {
            spdlog::debug("{} catched from inventory", weap->GetFullName());
            success = true;
        } else spdlog::error("could not find the {}", weap->GetFullName());

        if (success) {
            if (ExtraDataUtil::GetInventoryEntry(callerActor, weap)) {
                callerParent->SetSkipEquipAnim(true);
                callerParent->SetUnequipWhenAnimEnds(false);
                ObjectUtil::Actor::EquipItem(callerActor, weap, callerParent->GetSkipEquipAnim(), 1U, true, false, false);
                ObjectUtil::Actor::ResetEquipAnimationAfter(100, callerActor);
                RE::ShakeCamera(0.3f, position, 0.5f);
                if (callerParent->GetSkipEquipAnim()) callerParent->SetSkipEquipAnim(false);


                SetThrowState(ThrowState::kArrived);

                auto assets = Assets::GetSingleton();
                auto& rHandBone = callerParent->GetRHandBone();
                soundData.PlayCatchingSounds(reinterpret_cast<RE::NiAVObject**>(&rHandBone));
                if (auto handEffect = assets->VFXeffects.handFrost; handEffect) 
                    callerActor->ApplyArtObject(handEffect, 1.f, nullptr, false, false, rHandBone);

                if (Config::UsePrecisionTrails) {
                    auto animObjectR = callerParent->GetAnimObjectRBone();
                    auto weaponBone = callerParent->GetWeaponBone();
                    if (animObjectR && animObjectR->AsNode() && weaponBone && weaponParentNode) {
                        if (animObjectR == weaponParentNode->parent) {
                            animObjectR->AsNode()->DetachChild(weaponParentNode.get());
                        }
                        animObjectR->AsNode()->AttachChild(weaponParentNode.get());
                        weaponParentNode->local.translate = RE::NiPoint3();
                        auto& localRotation = weaponParentNode->local.rotate;
                        const auto& desiredParentWorldRotate = weaponBone->world.rotate;
                        const auto& parentWorldRotate = animObjectR->world.rotate;
                        localRotation = parentWorldRotate.Transpose() * desiredParentWorldRotate;
                    }
                }

                ResetCharge(enchMag, defaultEnchMag, true);
            }
        }

        if (stuckedBone)   stuckedBone.reset();
        if (stuckedActor)  stuckedActor.reset();
        lastHitActors.clear();
        lastHitForms.clear();
        trailRemoveUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);
    }
    caller = nullptr;
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
                if (!isTargetActor && lastHitForms.Contains(target->GetHandle())) isSameTarget = true;
                else if (victim && shooter && victim != shooter) {
                    if (lastHitActors.Contains(victim->GetHandle())) isSameTarget = true;
                    if (!victim->IsDead()) {
                        if (isHoming) {
                            auto homingState = GetCurrentState() ? GetCurrentState()->As<HomingState>() : nullptr;
                            if (homingState && victim == homingState->GetNextTarget() && rtData.livingTime > 0.5f) {
                                homingState->OnHit(victim);
                                spdlog::debug("{} is hit to {} ({:8x}) and passed through while homing", projectile->GetName(), target ? target->GetName() : "NULL",  target ? target->formID : 0x0);
                            }
                        } else if (isArriving && !isSameTarget) {
                            auto arrivingState = GetCurrentState() ? GetCurrentState()->As<ArrivingState>() : nullptr;
                            if (arrivingState && victim == arrivingState->GetNextTarget() && rtData.livingTime > 0.5f) {
                                arrivingState->OnHit(victim);
                                spdlog::debug("{} is hit to {} ({:8x}) and passed through while arriving", projectile->GetName(), target ? target->GetName() : "NULL",  target ? target->formID : 0x0);
                            }
                        } //else if (isPenetrating && !isSameTarget) {
                        //    ObjectUtil::Actor::ProcessHitImpl(shooter, victim, projectile, false);
                        //}

                        if (!isArriving) stuckedActor = victim->GetHandle();

                        if (weap && ench && ench->effects[0])
                            ObjectUtil::Enchantment::ChargeInventoryWeapon(shooter, weap, -ench->effects[0]->effectItem.magnitude);

                        if (!isArriving && throwerParent) {
                            shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);

                            throwerParent->RestoreRage(Config::RageBuffRate->value * 0.25f, true);
                        }
                    }
                }

                if (isSameTarget || isArriving || isHoming) {
                    return true;
                } else if (weaponParentNode && weaponParentNode->parent) {
                    _hitTransformApplied = true;
                    if (rotationType == RotationType::kNone) {
                        break;
                    }
                    auto& localRotate = weaponParentNode->local.rotate;
                    RE::NiPoint3 angles; localRotate.ToEulerAnglesXYZ(angles);
                    auto surfaceNormal = MathUtil::Algebra::MatrixToVector3(point.contact.separatingNormal); surfaceNormal.Unitize();
                    const auto dot = direction.Dot(surfaceNormal);
                    const auto localTarget = direction - dot * surfaceNormal;
                    angles.z = Config::HitRotationZ;
                //    angles.z = std::atan2(localTarget.x, localTarget.y) + Config::HitRotationZ;   // wrong
                    localRotate.SetEulerAnglesXYZ(angles);
                    hitLocalRotation = localRotate;

                    if (projectile && projectileModel) {
                        projectile->Update3DPosition(true);
                        MathUtil::Algebra::SetRotationMatrix(projectileModel->local.rotate, -direction.x, direction.y, direction.z);
                    }
                    auto setProjectileRotationFast = [this]() {
                        if (projectile && projectileModel) {
                            projectile->Update3DPosition(true);
                            MathUtil::Algebra::SetRotationMatrix(projectileModel->local.rotate, -direction.x, direction.y, direction.z);
                        }
                    };
                //    AsyncUtil::TaskManager::GetSingleton().AddPeriodicTask(1.69f, 0.5, setProjectileRotationFast);

                    if (Config::DebugModeOpen) {
                        constexpr float duration = 6.f;
                        const auto hitPoint = MathUtil::Algebra::HkVectorToNiPoint(point.contact.position) / havokWorldScale;
                        const auto directionPoint = hitPoint + 60.f * direction;
                        AsyncUtil::TaskManager::GetSingleton().AddPerFrameTask(
                            duration,
                            [hitPoint, directionPoint]() {
                                DebugRenderer::DrawLine(hitPoint, directionPoint, GRAY);
                            }
                        );
                        const auto surfaceNormalPoint = hitPoint + 30.f * MathUtil::Algebra::HkVectorToNiPoint(surfaceNormal);
                        AsyncUtil::TaskManager::GetSingleton().AddPerFrameTask(
                            duration,
                            [hitPoint, surfaceNormalPoint]() {
                                DebugRenderer::DrawLine(hitPoint, surfaceNormalPoint, GOLD);
                            }
                        );
                        const auto hitRotationPoint = hitPoint - 30.f * (localRotate * frontVec3);
                        AsyncUtil::TaskManager::GetSingleton().AddPerFrameTask(
                            duration,
                            [hitPoint, hitRotationPoint]() {
                                DebugRenderer::DrawLine(hitPoint, hitRotationPoint, ICEBLUE);
                            }
                        );
                    }
                    spdlog::info("hit rotation is {}, {}, {}", angles.x, angles.y, angles.z);
                }
                break;
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
    const bool isTargetActor = a_target ? a_target->formType == RE::FormType::ActorCharacter : false;
    const auto victim = isTargetActor ? a_target->As<RE::Actor>() : nullptr;
    auto& rtData = projectile->GetProjectileRuntimeData();
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

    bool skipIt = false;
    bool stopSounds = true;

    if (isTargetActor) lastHitActors.Push(victim->GetHandle());
    else if (a_target) lastHitForms.Push(a_target->GetHandle());
    else skipIt = true; //  the target is not an actor or a form, probably the ground or an inpenetrable thing.

    if (isPenetrating && skipIt) {
        a_impactData->impactResult = RE::ImpactResult::kBounce;
        isPenetrating = false;
    } else if (isPenetrating) {
        a_impactData->impactResult = RE::ImpactResult::kBounce;
        stopSounds = true;
        spdlog::debug("{} is hit to {} ({:8x}) but not passed through!", projectile->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
    } else if (a_impactData->impactResult == RE::ImpactResult::kStick) {  //  let it try to stick if it can
        //
    } else if (isTargetActor) { //  don't let it stick to actors but let it do whatever it wants.
    //    for (auto& impact : rtData.impacts) if (impact) impact->impactResult = RE::ImpactResult::kBounce;
    //    a_impactData->impactResult = RE::ImpactResult::kBounce;
    }

    const bool itWillStick = a_impactData->impactResult == RE::ImpactResult::kStick;
    const bool itWillBounce = a_impactData->impactResult == RE::ImpactResult::kBounce;
    if (itWillStick) {
        if (victim) {
            auto victimAVO = victim->AsActorValueOwner();
            if (victimAVO && rtData.weaponDamage > (victimAVO->GetActorValue(RE::ActorValue::kHealth) / 10.f)) {
                SKSE::GetTaskInterface()->AddTask([this, victim, &rtData]() {
                    ObjectUtil::Actor::PushActorAway(victim, rtData.power, direction);
                });
            }
            spdlog::debug("victim health: {} damage: {}!", victimAVO->GetActorValue(RE::ActorValue::kHealth), rtData.weaponDamage);
            stuckedActor = victim->GetHandle();
        }
        stuckedBone.reset(a_impactData->damageRootNode);
    //    for (auto& impact : rtData.impacts) if (impact) impact->impactResult = RE::ImpactResult::kStick;
        spdlog::debug("{} is sticked to {}!", projectile->GetName(), stuckedBone ? stuckedBone->name : "NULL");
    } else if (itWillBounce) {
    //    for (auto& impact : rtData.impacts) if (impact) impact->impactResult = RE::ImpactResult::kBounce;
        spdlog::debug("{} is bounced from {}!", projectile->GetName(), a_impactData->damageRootNode ? a_impactData->damageRootNode->name : "NULL");
    } else {
        a_impactData->impactResult = RE::ImpactResult::kBounce;
    }

    if (stopSounds)
        GetSoundManager().FadeThrowingLoopSounds(369);

    switch (a_impactData->impactResult) {
    case RE::ImpactResult::kBounce:  projState = ProjectileState::kHavok;   break;
    case RE::ImpactResult::kStick:   projState = ProjectileState::kStucked; break;
    case RE::ImpactResult::kDestroy: projState = ProjectileState::kNone;    break;
    case RE::ImpactResult::kImpale:  projState = ProjectileState::kNone;    break;
    case RE::ImpactResult::kNone:    projState = ProjectileState::kNone;    break;
    default: ProjectileState::kNone; break;
    }

    auto setProjectileRotation = [this]() {
        if (projectileModel)
            MathUtil::Algebra::SetRotationMatrix(projectileModel->local.rotate, -direction.x, direction.y, direction.z);
    };

    auto setProjectileRotationFast = [this]() {
        if (projectile && projectileModel) {
            projectile->Update3DPosition(true);
            MathUtil::Algebra::SetRotationMatrix(projectileModel->local.rotate, -direction.x, direction.y, direction.z);
        }
    };
    //AsyncUtil::TaskManager::GetSingleton().AddPeriodicTask(2.69f, 0.2, setProjectileRotationFast);
    //AsyncUtil::TaskManager::GetSingleton().AddPerFrameTask(2.69f, setProjectileRotationFast);
}
void SmartRelicWeapon::OnTrailDelete()
{
    if (weaponParentNode) {
        if (weaponParentNode->parent) {
            weaponParentNode->parent->DetachChild(weaponParentNode.get());
            weaponParentNode->parent = nullptr;
        }
        weaponParentNode.reset();
    }
}

void SmartRelicWeapon::Update() 
{
    ThrowableRelicWeapon::Update();
    if (delayedCallUpdate.IsTimeToUpdate() && caller) {
        caller->CallWeapon(this);
    }
    if (Config::DebugModeOpen) {
        if (replacedProjectileModel) {
            const auto& world = replacedProjectileModel->world;
            auto forward = world.rotate * frontVec3; forward.Unitize();
            const auto& start = world.translate;
            const auto& end = world.translate + forward * 30.f / 2.f;
            DebugRenderer::DrawPoint(start, 5.f, RED);
            DebugRenderer::DrawPoint(end, 5.f, RED);
            DebugRenderer::DrawLine(start, end, RED);
        }
        if (projectileModel) {
            const auto& world = projectileModel->world;
            auto forward = world.rotate * frontVec3; forward.Unitize();
            const auto& start = world.translate;
            const auto& end = world.translate + forward * 30.f / 2.f;
            DebugRenderer::DrawPoint(start, 5.f, CYAN);
            DebugRenderer::DrawPoint(end, 5.f, CYAN);
            DebugRenderer::DrawLine(start, end, CYAN);
        }
        if (weaponParentNode) {
            const auto& world = weaponParentNode->world;
            auto forward = world.rotate * frontVec3; forward.Unitize();
            const auto& start = world.translate;
            const auto& end = world.translate + forward * 30.f / 2.f;
            DebugRenderer::DrawPoint(start, 5.f, YELLOW);
            DebugRenderer::DrawPoint(end, 5.f, YELLOW);
            DebugRenderer::DrawLine(start, end, YELLOW);
        }
    }
}
void SmartRelicWeapon::UpdateProjectile(RE::Projectile* a_projectile, float a_delta) 
{
    ThrowableRelicWeapon::UpdateProjectile(a_projectile, a_delta);
    if (currentState) {
        auto const state  = currentState.get();
        const auto status = currentState->Update(a_delta);
        if (status > RelicWeaponState::Status::kRunning && currentState.get() == state) {
            currentState->Exit();
            currentState.reset();
        }
    }
}
