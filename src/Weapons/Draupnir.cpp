#include "Draupnir.h"

bool Draupnir::Initialize()
{
    bool found = true;
    auto dataHandler = RE::TESDataHandler::GetSingleton();
    DraupnirSpearProjBaseL      = dataHandler->LookupForm<RE::BGSProjectile>(0x802, Config::DraupnirModESP);
    DraupnirsCallProjBaseL      = dataHandler->LookupForm<RE::BGSProjectile>(0x818, Config::DraupnirModESP);
    SpellDraupnirProjL          = dataHandler->LookupForm<RE::SpellItem>    (0x800, Config::DraupnirModESP);
    SpellDraupnirsCallProjL     = dataHandler->LookupForm<RE::SpellItem>    (0x805, Config::DraupnirModESP);
    DraupnirExplosion           = dataHandler->LookupForm<RE::BGSExplosion> (0x809, Config::DraupnirModESP);
    DraupnirStuckedFX           = dataHandler->LookupForm<RE::BGSArtObject> (0x819, Config::DraupnirModESP);
    if (DraupnirSpearProjBaseL)
            spdlog::debug("Draupnir Spear projectile is {}", DraupnirSpearProjBaseL->GetName());
    else     {spdlog::error("Can't find Draupnir Spear projectile"); found = false;}
    if (DraupnirsCallProjBaseL)
            spdlog::debug("Draupnir's call projectile is {}", DraupnirsCallProjBaseL->GetName());
    else     {spdlog::error("Can't find Draupnir Spear projectile"); found = false;}
    if (SpellDraupnirProjL)
            spdlog::debug("Draupnir Spear projectile spell is {}", SpellDraupnirProjL->GetName());
    else     {spdlog::error("Can't find Draupnir Spear projectile spell"); found = false;}
    if (SpellDraupnirsCallProjL)
            spdlog::debug("Draupnir's call spell is {}", SpellDraupnirsCallProjL->GetName());
    else     {spdlog::error("Can't find Draupnir Spear projectile spell"); found = false;}
    if (DraupnirExplosion)
            spdlog::debug("Draupnir Spear explosion is {}", DraupnirExplosion->GetName());
    else     {spdlog::error("Can't find Draupnir Spear explosion"); found = false;}

    DefaultDraupnirModel = DraupnirSpearProjBaseL->model;

    return found;
}
void Draupnir::Update()
{
    if (projectileUpdate.IsTimeToUpdate()) {
        if (runtimeData.projectileModel && runtimeData.projectile && runtimeData.projectile->Get3D() && runtimeData.weaponModelCopy && runtimeData.projectileModel == runtimeData.projectile->Get3D()) {
        //    const RE::BSFixedString stuckedModelNodeName = "DraupnirProjectile";
        //    auto stuckedModel = runtimeData.projectileModel->GetObjectByName(stuckedModelNodeName);
        //    stuckedModel->GetFlags() |= RE::NiAVObject::Flag::kHidden;

        //    const RE::BSFixedString stuckedModelLightFadeNodeName = "LightSpellProjectile";
        //    auto stuckedLight = runtimeData.projectileModel->GetObjectByName(stuckedModelLightFadeNodeName);
        //    stuckedLight->GetFlags() |= RE::NiAVObject::Flag::kHidden;

            auto node = runtimeData.projectileModel->AsNode();

            auto cloneModel = runtimeData.weaponModelCopy.get()->Clone();
            auto cloneNode = cloneModel ? cloneModel->AsNode() : nullptr;
            runtimeData.replacedProjectileModel.reset(cloneNode);

            if (node) {
                node->AttachChild(runtimeData.replacedProjectileModel.get(), false);
                projectileUpdate.Done();
                trailUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);
                spdlog::debug("draupnir model changed!");
            } else spdlog::warn("draupnir node null");
        } else spdlog::warn("proj or proj->Get3D2() null");
    }
    if (Config::DrawTrails) {
        if (trailUpdate.IsTimeToUpdate()) {
            auto bone = runtimeData.replacedProjectileModel;
            if (bone) {
                const float intensity = 2.f;
                const auto meshOverride = Config::TrailModelPathDef;
                float length = bone->worldBound.radius;
                ObjectUtil::Capsule capsule;
                ObjectUtil::Node::GetCapsuleParams(bone->AsNode(), capsule);
                float capsuleLength = capsule.a.GetDistance(capsule.b);
                length = length > capsuleLength ? length : capsuleLength;
                float scale = fmax(length, capsule.radius) * 0.01f;
                float tipOffset = length;
                trailData = TrailData(meshOverride, intensity);

                if (Config::UsePrecisionTrails && (Config::IsPrecisionInstalled || APIs::precision || APIs::Request())) {
                    trailUpdate.Done();
                    trailData.transformOverride.additionalRotation = RE::NiMatrix3(0.f, 0.f, -NI_HALF_PI);
                    trailData.transformOverride.scale = bone->worldBound.radius * 0.01f;
                    auto node = RE::NiNode::Create(0);
                    bone->AttachChild(node, false);
                    APIs::precision->AddTrailEffect(
                        node, 
                        RE::PlayerCharacter::GetSingleton()->GetParentCell(), 
                        trailData.trailOverride, 
                        trailData.transformOverride);
                }
            }
        }
    }
    if (explosionsStarted) {
        if (AsyncUtil::GameTime::GetEngineTime() >= nextExplosionTime) {
            RE::ProjectileHandle pHandle;
            TriggerExplosions(explosionDelay, 15.f, &pHandle);
        }
    }
    if (draupnirRainStarted) {
        if (AsyncUtil::GameTime::GetEngineTime() >= nextLaunchTime) {
            RainOfDraupnir();
        }
    }
}

bool Draupnir::PreThrow()
{
    bool result = false;

    trailRemoveUpdate.Done();

    auto throwerActor = runtimeData.thrower ? runtimeData.thrower->GetActor() : nullptr;
    if (!throwerActor) {spdlog::error("Draupnir::PreThrow - thrower actor is null"); return result;}

    bool isDraupnirSpear = runtimeData.thrower->GetRightHandRelic() == this;
    if (!isDraupnirSpear) return result;

    return result;
}
void Draupnir::PostThrow()
{
    if (runtimeData.projectile) {
        MeleeHitProjectileIDs.emplace_back(runtimeData.projectile->formID);
        spdlog::debug("melee thrown draupnir form id: {}", runtimeData.projectile->formID);
    }
}

void Draupnir::ArtilleryOfTheAncients(const float a_delay, const float a_duration)
{
    if (SpellDraupnirProjL) {
        nextLaunchDelay = a_delay;
        nextLaunchTime = AsyncUtil::GameTime::GetEngineTime();
        lastLaunchTime = AsyncUtil::GameTime::GetEngineTime() + a_duration;
        draupnirRainStarted = true;
        spdlog::info("rain of draupnir triggered");
    }
}
void Draupnir::RainOfDraupnir()
{
    if (nextLaunchTime > lastLaunchTime) {
        draupnirRainStarted = false;
        spdlog::info("rain of draupnir done");
            return;
    }
    //  launch draupnir at a random top position of the nearest target
    const auto AnArchos = PlayerCharacter::GetSingleton();
    const std::vector<RE::Actor*> nearCombatTargets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(AnArchos, 666.f);
    const bool isThereNearTarget = (!nearCombatTargets.empty() && nearCombatTargets.front()) ? true : false;
    RE::NiPoint3 origin = isThereNearTarget ? nearCombatTargets.front()->GetPosition() : (AnArchos->GetPosition() + MathUtil::Algebra::GetForwardVector(AnArchos) * 360.f);
    origin += RE::NiPoint3(MathUtil::Algebra::GenerateRandomFloat(-100.f, 100.f), MathUtil::Algebra::GenerateRandomFloat(-100.f, 100.f), 2000.f);
    RE::ProjectileHandle pHandle;
    RE::Projectile::ProjectileRot pRot{};
    pRot.x = PI2 + MathUtil::Algebra::GenerateRandomFloat(-0.018f, 0.018f);
//    spdlog::debug("aim angle: {} heading angle: {}", pRot.x, pRot.z);
    RE::Projectile::LaunchData lData(AnArchos, origin, pRot, SpellDraupnirProjL);

    Throw(RotationType::kNone, pRot, origin);

    nextLaunchTime = AsyncUtil::GameTime::GetEngineTime() + nextLaunchDelay;
}
void Draupnir::Call(const float a_damage, const float a_force)
{
    if (SpellDraupnirsCallProjL && DraupnirExplosion) {
        DraupnirExplosion->data.force = a_force * explosionMagnitude;
        StartExplosions(Config::DraupnirExplosionsInterval);
    }
}
void Draupnir::AddSpearHit(RE::NiNode* bone, RE::Actor* actor)
{
    if (bone) {
        if (spearHits.size() > Config::DraupnirSpearCount) {
            auto& [oldBone, oldTarget, oldProjectile] = spearHits.front();
            if (oldProjectile) oldProjectile->GetProjectileRuntimeData().flags |= pFlag::kDestroyed;
            spearHits.erase(spearHits.begin());
        }   spearHits.emplace_back(bone, actor, nullptr);
    }
}
void Draupnir::AddSpearHit(RE::Projectile* a_proj)
{
    if (a_proj) {
        if (spearHits.size() > Config::DraupnirSpearCount) {
            auto& [oldBone, oldTarget, oldProjectile] = spearHits.front();
            if (oldProjectile) oldProjectile->GetProjectileRuntimeData().flags |= pFlag::kDestroyed;
            spearHits.erase(spearHits.begin());
        }   spearHits.emplace_back(nullptr, nullptr, a_proj);
    }
}
void Draupnir::StartExplosions(const float a_delay)
{
    if (spearHits.empty() && explosionsStarted) {
        spdlog::info("there is no thrown explodable draupnir spear");
        explosionsStarted = false;
        return;
    }
    explosionDelay = a_delay;
    nextExplosionTime = AsyncUtil::GameTime::GetEngineTime();// + a_delay;
    explosionsStarted = true;
    spdlog::info("draupnir explosions triggered");
}
void Draupnir::TriggerExplosions(float a_delay, float a_force, RE::ProjectileHandle* a_pHandle)
{
    if (spearHits.empty()) {
        if (AsyncUtil::GameTime::GetEngineTime() > timeToDoneExplosions) {
            explosionsStarted = false;
            currentHitIndex = 0;
            SetExplosionMagnitude(1.f);
            spdlog::info("draupnir explosion chain finished");
        } //spdlog::info("all draupnir explosions done");
    } else {
        auto& [bone, target, proj] = spearHits.front();
        currentHitIndex++;
        if (bone && target && a_pHandle) {
            TriggerExplosionAtLocation(bone.get(), a_pHandle, target.get());
            target->RemoveExtraArrows3D();
            spdlog::info("{}. draupnir explosion done", currentHitIndex);
        } else if (proj && a_pHandle) {
            TriggerExplosionAtLocation(proj.get(), a_pHandle);
            spdlog::info("{}. draupnir explosion done", currentHitIndex);
        } else spdlog::info("{}. draupnir explosion is empty!", currentHitIndex);
        spearHits.erase(spearHits.begin());
        nextExplosionTime = AsyncUtil::GameTime::GetEngineTime() + a_delay;
        timeToDoneExplosions = nextExplosionTime + 0.3f;
    }
}
void Draupnir::TriggerExplosionAtLocation(RE::Projectile* a_proj, RE::ProjectileHandle* a_pHandle)
{
    RE::Projectile::LaunchData lData(RE::PlayerCharacter::GetSingleton(), a_proj->data.location, RE::Projectile::ProjectileRot(), SpellDraupnirsCallProjL);
    RE::Projectile::Launch(a_pHandle, lData);
    a_proj->GetProjectileRuntimeData().flags |= pFlag::kDestroyed;
//    a_proj->GetProjectileRuntimeData().explosionTimer = 0;    //  this method just works but explodes at the first hit position, not current 3d position.
}
void Draupnir::TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::ProjectileHandle* a_pHandle, RE::Actor* a_target)
{
    RE::Projectile::LaunchData lData(GetThrowerActor(), a_bone->world.translate, RE::Projectile::ProjectileRot(), SpellDraupnirsCallProjL);
    lData.desiredTarget = a_target;
    RE::Projectile::Launch(a_pHandle, lData);
    auto explosionDamage = runtimeData.projectile ? runtimeData.projectile->GetProjectileRuntimeData().weaponDamage : damage;
    explosionDamage *= explosionMagnitude;
    FenixUtils::stagger(std::clamp(explosionDamage / 10.f, 0.1f, 100.f), a_target, GetThrowerActor());
    if (auto targetAVO = a_target->AsActorValueOwner(); targetAVO)
        targetAVO->RestoreActorValue(RE::ActorValue::kHealth, -explosionDamage);
}

void Draupnir::ReplaceStickedProjectileModel(RE::Projectile* a_proj)
{
    if (a_proj && a_proj->Get3D() && runtimeData.projectileModel) {
        auto projModel = a_proj->Get3D();
        auto projNode = projModel ? projModel->AsFadeNode() : nullptr;
        auto copyModelObj = runtimeData.projectileModel->Clone();
        auto copyModel = copyModelObj ? copyModelObj->AsNode() : static_cast<RE::NiAVObject*>(copyModelObj);
        if (projNode && copyModel) {
            const RE::BSFixedString stuckedModelNodeName = "DraupnirSpearBlade";
            auto stuckedModel = copyModel->GetObjectByName(stuckedModelNodeName);
            if (stuckedModel) {
                stuckedModel->flags &= RE::NiAVObject::Flag::kHidden;
                projNode->AttachChild(stuckedModel, false);
                spdlog::debug("draupnir model replaced with stucked model");
            }

            const RE::BSFixedString stuckedModelLightFadeNodeName = "LightSpellProjectile";
            auto stuckedLight = copyModel->GetObjectByName(stuckedModelLightFadeNodeName);
            if (stuckedLight) {
                stuckedLight->AsFadeNode()->flags &= RE::NiAVObject::Flag::kHidden;
                projNode->AttachChild(stuckedLight, false);
                spdlog::debug("added the light model to stucked draupnir model");
            }

            if (runtimeData.replacedProjectileModel && runtimeData.replacedProjectileModel->parent) {
                runtimeData.replacedProjectileModel->parent->DetachChild(runtimeData.replacedProjectileModel.get());
                runtimeData.replacedProjectileModel.reset();
            }
        } else spdlog::warn("projectile node is null");
    } else spdlog::warn("projectile or projectile model is null");
}

void Draupnir::PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) 
{
    auto missileProjectile = runtimeData.projectile ? runtimeData.projectile->As<RE::MissileProjectile>() : nullptr;
    if (missileProjectile && a_impactData) {
        const bool isTargetActor = a_target ? a_target->formType == RE::FormType::ActorCharacter : false;
        const auto victim = isTargetActor ? a_target->As<RE::Actor>() : nullptr;
        auto& rtData = missileProjectile->GetProjectileRuntimeData();
        auto& missileRTD = missileProjectile->GetMissileRuntimeData();
        auto shooter = rtData.shooter ? rtData.shooter.get()->As<RE::Actor>() : nullptr;
        const auto projBase = missileProjectile->GetProjectileBase();
        bool isDraupnirSpear = runtimeData.thrower->GetRightHandRelic() == this;

        if (projBase == DraupnirsCallProjBase) {
            missileRTD.impactResult = RE::ImpactResult::kImpale;
            a_impactData->impactResult = RE::ImpactResult::kImpale;
        }
        else if (isDraupnirSpear) {
        //    bool isMelee = false;
        //    if (!Draupnir::MeleeHitProjectileIDs.empty())
        //        for (auto ID : Draupnir::MeleeHitProjectileIDs) {
        //            if (ID == missileProjectile->formID) {
        //            //    missileProjectile->Set3D(Draupnir::spearModel, false);
        //                isMelee = true;
        //            }
        //        }
        //    if (!isMelee) Draupnir::spearModel = missileProjectile->Get3D();
            auto impactDataP = *rtData.impacts.begin();
            impactDataP->impactResult = RE::ImpactResult::kStick;
            missileRTD.impactResult = RE::ImpactResult::kStick;
            a_impactData->impactResult = RE::ImpactResult::kStick;
            const bool itWillStick = (missileRTD.impactResult == RE::ImpactResult::kStick);
            spdlog::debug("draupnir is will {}", itWillStick ? "stick" : "bounce");
            if (victim && a_impactData->damageRootNode && itWillStick) {
                AddSpearHit(a_impactData->damageRootNode, victim);
                spdlog::debug("spear stucked to {} of {}!", a_impactData->damageRootNode->name, victim->GetName());
            } else {
                AddSpearHit(missileProjectile);
                spdlog::debug("draupnir's target is not an actor.");
            }
        }
    }
}
