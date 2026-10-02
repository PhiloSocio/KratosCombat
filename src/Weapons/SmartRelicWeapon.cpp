#include "SmartRelicWeapon.h"
#include "Weapons/States/ArrivingState.h"
#include "Weapons/States/HomingState.h"
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
    return ArrivingWeaponDummyAmmo != nullptr;
}

void SmartRelicWeapon::GetPosition(RE::NiPoint3& a_point)
{
    if (replacedProjectileModel) {
        transformW = GetWorldTransform();
        a_point = transformW.translate * 70.f;
    //    a_point = replacedProjectileModel->worldBound.center;
        spdlog::debug("leviathan coming from weapon projectileModel location.");
    } else spdlog::debug("we can't get leviathan's weapon projectileModel!");

    if (stuckedBone) {
        a_point = replacedProjectileModel ? transformW.translate : stuckedBone->world.translate;
        stuckedBone.reset();

        if (stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
            ObjectUtil::Projectile::DeleteAnExtraArrow(stuckedActor, projectileModel);
#else
            stuckedActor->RemoveExtraArrows3D();
#endif
            spdlog::debug("levi stucked actor's extra arrows removed");
            stuckedActor.reset();
        } else spdlog::debug("levi not stucked anybody");
    } else spdlog::debug("levi not stucked any bone");

    auto callerActor = caller ? caller->GetActor() : nullptr;
    if (!callerActor) return;

    if (GetThrowState() == ThrowState::kThrowable) {
        if (auto backWeaponSheathe = callerActor->GetNodeByName("WeaponBack"); backWeaponSheathe) {
            const auto& backSheatheTransform = backWeaponSheathe->world;
            a_point = backSheatheTransform.translate;
            const auto& rightDir = backSheatheTransform.rotate * rightVec3;
            const auto& backDir = backSheatheTransform.rotate * backVec3;
            velocity = (0.69f * rightDir + 0.31f * backDir) * 2400.f;
            spdlog::debug("levi is coming from your back sheathe");
        }
    } else {
        auto pcPos = callerActor->GetPosition();
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

void SmartRelicWeapon::Call(BaseActor* a_caller, const bool a_justDestroy, std::optional<float> a_delay)
{
    caller = a_caller;
    if (caller && caller->IsValid() && weap) {
        spdlog::debug("Levi is calling...");
        projectileUpdate.Done();

        trailUpdate.Done();
        trailRemoveUpdate.Done();

        auto callerActor = caller ? caller->GetActor() : nullptr;
        if (callerActor)
            callerActor->SetGraphVariableBool("bLeviInCatchRange", false);

        if (projectileModel) {
            transformPW = projectileModel->world;
            transformPL = projectileModel->local;
        }
        projectileModel = nullptr;
    //    if (replacedProjectileModel) {
    //        if (projState == ProjectileState::kHavok && replacedProjectileModel->collisionObject && replacedProjectileModel->collisionObject->AsBhkRigidBody()) {
    //            RE::hkTransform rbTransform;
    //            replacedProjectileModel->collisionObject->AsBhkRigidBody()->GetTransform(rbTransform);
    //            RE::NiTransform niTransform;
    //            niTransform.translate = MathUtil::Algebra::HkVectorToNiPoint(rbTransform.translation);
    //            niTransform.rotate = MathUtil::Algebra::HKMatrixToNiMatrix(rbTransform.rotation);
    //            transformW = niTransform;
    //            transformL = niTransform;
    //        } else {
    //            transformW = replacedProjectileModel->world;
    //            transformL = replacedProjectileModel->local;
    //        }
    //    }

        soundData.FadeThrowingLoopSounds(369);

        if (a_justDestroy) {
            if (stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
                ObjectUtil::Projectile::DeleteAnExtraArrow(stuckedActor, projectileModel);
#else
                stuckedActor->RemoveExtraArrows3D();
#endif
                spdlog::debug("levi stucked actor's extra arrows removed");
                stuckedActor = nullptr;
            } else {
                spdlog::debug("levi not stucked anybody");
            }
            return;
        }

        auto rHandBone = caller->GetRHandBone();
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
                } else  spdlog::debug("levi is already destroyed");
            }

            if (auto callerActor = caller->GetActor(); callerActor && ArrivingWeaponDummyAmmo && ArrivingWeaponDummyProjectile) {
                RE::NiPoint3 startPoint = position;
                GetPosition(startPoint);
                projectileRotation = MathUtil::Algebra::VectorToPitchYaw(direction);
                SetThrowState(ThrowState::kNone);
                Throw(dynamic_cast<Thrower*>(caller), RotationType::kSpinVertical, projectileRotation, startPoint);
    /*
                RE::Projectile::LaunchData lData(AnArchos, startPoint, projectileRotation, ArrivingWeaponDummyAmmo, weap);

                lData.noDamageOutsideCombat = true; //  can be an option
                lData.weaponSource = weap;
    #ifdef EXPERIMENTAL_THROWPOISON
                lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(AnArchos, false);
    #endif
                if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
                    lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

                projectileHandle = RE::Projectile::Launch(&pHandle, lData);
                projectile = pHandle.get().get();

                projectileUpdate.RegisterForUpdate(0.0f, false);
    */
                SetState(RelicWeaponState::Type::kArriving);
                spdlog::info("weapon arriving...");
            } else {
                spdlog::warn("WEIRD SpellLeviProjA is nullptr!");
            }

        } else if (projectile) {
            spdlog::debug("the projectile is already active, changing its state to arriving state...");
            SetThrowState(ThrowState::kNone);
            SetState(RelicWeaponState::Type::kArriving);
        }
    } else {spdlog::warn("WEIRD you don't have the axe for calling!!");}
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
        if (a_justDestroy) return;
    }

    auto callerActor = caller ? caller->GetActor() : nullptr;
    if (callerActor && !caller->GetRightHandRelic()) {
        callerActor->SetGraphVariableBool("bLeviInCatchRange", true);

        SetThrowState(ThrowState::kArrived);

        auto assets = Assets::GetSingleton();
        auto rHandBone = caller->GetRHandBone();
        if (auto handEffect = assets->VFXeffects.handFrost; handEffect) 
            callerActor->ApplyArtObject(handEffect, 1.f, nullptr, false, false, rHandBone);

        soundData.FadeArrivingNearSounds(469);
    //    soundData.FadeArrivingLoopSounds(469);
        soundData.StopArrivingLoopSounds(*g_deltaTimeRealTime * 1200.f);
        soundData.PlayCatchingSounds(rHandBone);

        if (weap && GetWeaponContainer()) {
            caller->SetSkipEquipAnim(true);
            caller->SetUnequipWhenAnimEnds(false);
            caller->DoAction(ActionType::kWeaponCharge);
            GetWeaponContainer()->RemoveItem(weap, 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, callerActor);
            ObjectUtil::Actor::EquipItem(callerActor, weap, caller->GetSkipEquipAnim());//, 1U, true, false, false, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, callerActor);
            RE::ShakeCamera(0.3f, position, 0.5f);
            if (caller->GetSkipEquipAnim()) caller->SetSkipEquipAnim(false);
        } else spdlog::warn("you not have the leviathan axe");

        if (stuckedBone)   stuckedBone    = nullptr;
        if (stuckedActor)  stuckedActor   = nullptr;
        lastHitActors.clear();
        lastHitForms.clear();

        if (Config::UsePrecisionTrails) {
            if (caller->GetAnimObjectRBone() && caller->GetAnimObjectRBone()->AsNode() && replacedProjectileModel && replacedProjectileModel->parent) {
                caller->GetAnimObjectRBone()->AsNode()->AttachChild(replacedProjectileModel->parent);
                replacedProjectileModel->parent->local.translate = RE::NiPoint3();
                replacedProjectileModel->parent->local.rotate = replacedProjectileModel->parent->local.rotate * RE::NiMatrix3(PI2, 0.f, PI2);
            }
        }
        trailUpdate.Done();
        projectileModel = nullptr;
        trailRemoveUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);
    }
}

bool SmartRelicWeapon::OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) 
{
    const auto projBase = ThrowableWeaponDummyProjectile;
    if (projBase && projectile) {
        auto& rtData = projectile->GetProjectileRuntimeData();
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
                else if (victim && victim != shooter) {
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
        auto shooter = rtData.shooter ? rtData.shooter.get()->As<RE::Actor>() : nullptr;
        const auto projBase = missileProjectile->GetProjectileBase();

        trailUpdate.Done();

        if (!isPenetrating) {
            bool skipIt = false;
            bool stopSounds = true;
            const bool isHoming = IsHoming();
            const bool isArriving = IsArriving();

            if (isTargetActor)  {lastHitActors.insert(lastHitActors.begin(), victim->GetHandle()); if (lastHitActors.size() > 4) lastHitActors.pop_back();}
            else if (a_target)  {lastHitForms.insert(lastHitForms.begin(), a_target); if (lastHitForms.size() > 4) lastHitForms.pop_back();}
            else skipIt = true; //  the target is not an actor or a form, probably the ground or an inpenetrable thing.

            if (isArriving) {
                spdlog::debug("levi impacted to {} while arriving", a_target ? a_target->GetName() : "NULL");
                auto arrivingState = GetCurrentState() ? dynamic_cast<ArrivingState*>(GetCurrentState()) : nullptr;
                if (arrivingState && victim && victim == arrivingState->GetNextTarget())  arrivingState->GetTargets().erase(arrivingState->GetTargets().begin());
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                a_impactData->impactResult = RE::ImpactResult::kBounce;
                isPenetrating = true;
                Call(caller, false);
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
                spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
            } else if (isPenetrating && skipIt) {
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                a_impactData->impactResult = RE::ImpactResult::kBounce;
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                projState = ProjectileState::kHavok;
            } else if (isPenetrating) {
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kDestroy;
                a_impactData->impactResult = RE::ImpactResult::kDestroy;
                missileRTD.impactResult = RE::ImpactResult::kDestroy;
                projState = ProjectileState::kNone;
                auto pRot = MathUtil::Algebra::VectorToPitchYaw(direction);
                Throw(thrower, RotationType::kSpinLateral, pRot, position);
                stopSounds = false;
                spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
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
        //            ObjectUtil::Actor::PushActorAway(victim, projBase->data.force, rtData.linearVelocity);
        //        }
                stuckedActor.reset(victim);
                stuckedBone.reset(a_impactData->damageRootNode);
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kStick;
                projState = ProjectileState::kStucked;
                spdlog::debug("{} is sticked to {}!", projBase->GetName(), stuckedBone ? stuckedBone->name : "NULL");
            } else if (itWillBounce) {
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                projState = ProjectileState::kHavok;
                spdlog::debug("{} is bounced from {}!", projBase->GetName(), stuckedBone ? stuckedBone->name : "NULL");
            }
            if (stopSounds)
                GetSoundManager().FadeThrowingLoopSounds(369);
        }
    }
}
void SmartRelicWeapon::OnTrailDelete()
{
    if (caller && caller->GetAnimObjectRBone()) {
        caller->GetAnimObjectRBone()->AsNode()->DetachChild(replacedProjectileModel->parent);
    }
}

void SmartRelicWeapon::Update() 
{return;
    if (projectileUpdate.IsTimeToUpdate()) {
        if (projectileModel && projectile && projectile->Get3D() && weaponModelCopy && projectileModel == projectile->Get3D()) {
            const RE::BSFixedString rotatingBoneName = "BlastRadiusNode";
            auto animatedBone = projectileModel->GetObjectByName(rotatingBoneName);
            auto animatedNode = animatedBone ? animatedBone->AsNode() : nullptr;

            auto cloneModel = weaponModelCopy.get()->Clone();
            auto cloneNode = cloneModel ? cloneModel->AsNode() : nullptr;
            replacedProjectileModel.reset(cloneNode);

            if (animatedNode) {
                weaponParentNode->local = RE::NiTransform();
                animatedNode->AttachChild(replacedProjectileModel.get(), false);
            //    auto oldWorld = transformW;
            //    oldWorld.translate *= 70.f;
            //    oldWorld.scale = replacedProjectileModel.get()->world.scale;
            //    replacedProjectileModel.get()->local = ObjectUtil::Node::GetLocalTransform(replacedProjectileModel.get(), oldWorld);
                projectileUpdate.Done();
                trailUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);
                spdlog::debug("levi projectileModel changed!");
            } else spdlog::warn("animated node or levinode null");
        } else spdlog::warn("projectile or projectile->Get3D2() null");
    }
    if (soundData.arrivingLoopStopUpdate.IsTimeToUpdate()) {soundData.StopArrivingLoopSounds();}
    if (soundData.throwingLoopStopUpdate.IsTimeToUpdate()) {soundData.StopThrowingLoopSounds();}
    if (Config::DrawTrails) {
        AddTrail();
        FadeTrail();
    }
//    if (replacedProjectileModel) {
//        auto projNiTransform = replacedProjectileModel->world;
//        auto projBHKTransform = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(replacedProjectileModel.get());
//        auto projHKPTransform = ObjectUtil::Node::GetHavokHKPRigidBodyWorldTransform(replacedProjectileModel.get());
//        spdlog::debug(
//            "NI: ({}, {}, {})  BHK: ({}, {}, {})",
//            projNiTransform.translate.x,
//            projNiTransform.translate.y,
//            projNiTransform.translate.z,
//            projBHKTransform.translate.x * 70.f,
//            projBHKTransform.translate.y * 70.f,
//            projBHKTransform.translate.z * 70.f
//        );
    //    spdlog::debug(
    //        "NI - BHK: ({}, {}, {})",
    //        projNiTransform.translate.x - projBHKTransform.translate.x * 70.f,
    //        projNiTransform.translate.y - projBHKTransform.translate.y * 70.f,
    //        projNiTransform.translate.z - projBHKTransform.translate.z * 70.f
    //    );
//    }

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
