#include "Draupnir.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Thrower.h"
#include "Assets.h"
#include "settings.h"
#include "util.h"

Draupnir::~Draupnir()
{
    spdlog::debug("Draupnir destructed");
}
Draupnir::Draupnir(RE::TESBoundObject* a_object)
    : SmartRelicWeapon(a_object)
{
    Initialize();
}

bool Draupnir::Initialize()
{
    bool found = true;
    auto dataHandler = RE::TESDataHandler::GetSingleton();
    DraupnirsCallProjBaseL  = dataHandler->LookupForm<RE::BGSProjectile>(0x818, Config::DraupnirModESP);
    SpellDraupnirsCallProjL = dataHandler->LookupForm<RE::SpellItem>    (0x805, Config::DraupnirModESP);
    DraupnirExplosion       = dataHandler->LookupForm<RE::BGSExplosion> (0x809, Config::DraupnirModESP);
    DraupnirStuckedFX       = dataHandler->LookupForm<RE::BGSArtObject> (0x819, Config::DraupnirModESP);
    if (DraupnirsCallProjBaseL)
        spdlog::debug("Draupnir's call projectile is {}", DraupnirsCallProjBaseL->GetName());
    else {spdlog::error("Can't find Draupnir Spear projectile"); found = false;}
    if (SpellDraupnirsCallProjL)
        spdlog::debug("Draupnir's call spell is {}", SpellDraupnirsCallProjL->GetName());
    else {spdlog::error("Can't find Draupnir Spear projectile spell"); found = false;}
    if (DraupnirExplosion)
        spdlog::debug("Draupnir Spear explosion is {}", DraupnirExplosion->GetName());
    else {spdlog::error("Can't find Draupnir Spear explosion"); found = false;}

    ThrowableWeaponDummyProjectile->data.gravity = 2.34f;

    const auto assets = Assets::GetSingleton();
    soundData = 
        SoundDefinition{
            .chargingLoop0 = assets->soundEffects.chargeLeviLoop,
            .throwingStart = assets->soundEffects.throwLevi,
            .throwingLoop0 = assets->soundEffects.throwingLeviLoop0,
            .throwingLoop1 = assets->soundEffects.throwingLeviLoop1,
            .callStart     = assets->soundEffects.callLevi,
            .arrivingStart = assets->soundEffects.arrivingLeviStart,
            .arrivingLoop0 = assets->soundEffects.arrivingLeviLoop0,
            .arrivingLoop1 = assets->soundEffects.arrivingLeviLoop1,
            .arrivingLoop2 = assets->soundEffects.arrivingLeviLoop2,
            .arrivingNear  = assets->soundEffects.arrivingLeviNear,
            .catching      = assets->soundEffects.catchLevi
        };

    isCountless = true;

    return found;
}
void Draupnir::Update()
{
    SmartRelicWeapon::Update();
    if (explosionsStarted) {
        if (AsyncUtil::GameTime::GetEngineTime() >= nextExplosionTime) {
            RE::ProjectileHandle pHandle;
            TriggerExplosions(explosionDelay, 15.f, pHandle);
        }
    }
    if (draupnirRainStarted) {
        if (AsyncUtil::GameTime::GetEngineTime() >= nextLaunchTime) {
            RainOfDraupnir();
        }
    }
}

void Draupnir::OnMenuOpenCloseEvent(const bool a_opening)
{
    if (a_opening) {
        soundData.PauseAllLoopingSounds();
    } else {
        soundData.ContinueAllLoopingSounds();
    }
}
bool Draupnir::PreThrow()
{
    ThrowableWeaponDummyProjectile->data.speed *= Config::LeviThrowSpeedMult;
    return true;
}
void Draupnir::PostThrow()
{
    auto throwerActor = throwerParent ? throwerParent->GetActor() : nullptr;
    if (!throwerActor) {spdlog::error("Draupnir::Throw - thrower actor is null"); return;}

    gravity = 2.34f;
    gravity /= std::max(std::powf(throwingChargeDuration + 1.f, 3.f), 0.01f);
    gravity = std::max(gravity, 0.5f);

    trailData.trailOverride.baseColorOverride = trailData.GetColorByIndex(Config::TrailColorIndexLeviathan);

    if (projectile = projectileHandle.get().get(); projectile) {
        MeleeHitProjectiles.emplace_back(projectileHandle);
        spdlog::debug("melee thrown draupnir form id: {}", projectile->formID);
    }
}

void Draupnir::ArtilleryOfTheAncients(const float a_delay, const float a_duration)
{
    nextLaunchDelay = a_delay;
    nextLaunchTime = AsyncUtil::GameTime::GetEngineTime();
    lastLaunchTime = AsyncUtil::GameTime::GetEngineTime() + a_duration;
    draupnirRainStarted = true;
    spdlog::info("rain of draupnir triggered");
}
void Draupnir::RainOfDraupnir()
{
    if (!GetWielder()) return;

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
    RE::Projectile::LaunchData lData(AnArchos, origin, pRot, ThrowableWeaponDummyAmmo, weap);

    Throw(GetWielder()->GetCapabilityAs<Thrower>(), RotationType::kNone, pRot, origin);

    nextLaunchTime = AsyncUtil::GameTime::GetEngineTime() + nextLaunchDelay;
}
bool Draupnir::Call(Caller* a_caller, const bool a_justDestroy, std::optional<float> a_delay)
{
    if (SpellDraupnirsCallProjL && DraupnirExplosion) {
    //    DraupnirExplosion->data.force = a_force * explosionMagnitude;
        StartExplosions(Config::DraupnirExplosionsInterval);
    } return false;
}
void Draupnir::AddSpearHit(RE::NiNode* bone, RE::Actor* actor)
{
    if (bone) {
        if (spearHits.size() > Config::DraupnirSpearCount) {
            auto& [oldBone, oldTarget, oldProjectileHandle] = spearHits.front();
            if (const auto oldProjectile = oldProjectileHandle.get().get(); oldProjectile)
                oldProjectile->Kill();
            spearHits.erase(spearHits.begin());
        }   spearHits.emplace_back(bone, actor, RE::ProjectileHandle());
    }
}
void Draupnir::AddSpearHit(RE::Projectile* a_proj)
{
    if (a_proj) {
        if (spearHits.size() > Config::DraupnirSpearCount) {
            auto& [oldBone, oldTarget, oldProjectileHandle] = spearHits.front();
            if (const auto oldProjectile = oldProjectileHandle.get().get(); oldProjectile)
                oldProjectile->Kill();
            spearHits.erase(spearHits.begin());
        }   spearHits.emplace_back(nullptr, RE::ActorHandle(), a_proj);
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
    nextExplosionTime = AsyncUtil::GameTime::GetEngineTime();
    explosionsStarted = true;
    spdlog::info("draupnir explosions triggered");
}
void Draupnir::TriggerExplosions(float a_delay, float a_force, RE::ProjectileHandle& a_pHandle)
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
        if (bone && target && a_pHandle.get().get()) {
            TriggerExplosionAtLocation(bone.get(), a_pHandle, target.get().get());
            target.get()->RemoveExtraArrows3D();
            spdlog::info("{}. draupnir explosion done", currentHitIndex);
        } else if (proj && a_pHandle.get().get()) {
            TriggerExplosionAtLocation(proj.get().get(), a_pHandle);
            spdlog::info("{}. draupnir explosion done", currentHitIndex);
        } else spdlog::info("{}. draupnir explosion is empty!", currentHitIndex);
        spearHits.erase(spearHits.begin());
        nextExplosionTime = AsyncUtil::GameTime::GetEngineTime() + a_delay;
        timeToDoneExplosions = nextExplosionTime + 0.3f;
    }
}
void Draupnir::TriggerExplosionAtLocation(RE::Projectile* a_proj, RE::ProjectileHandle& a_pHandle)
{
    RE::Projectile::LaunchData lData(RE::PlayerCharacter::GetSingleton(), a_proj->data.location, RE::Projectile::ProjectileRot(), SpellDraupnirsCallProjL);
    RE::Projectile::Launch(&a_pHandle, lData);
    a_proj->GetProjectileRuntimeData().flags |= pFlag::kDestroyed;
//    a_proj->GetProjectileRuntimeData().explosionTimer = 0;    //  this method just works but explodes at the first hit position, not current 3d position.
}
void Draupnir::TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::ProjectileHandle& a_pHandle, RE::Actor* a_target)
{
    RE::Projectile::LaunchData lData(GetThrowerActor(), a_bone->world.translate, RE::Projectile::ProjectileRot(), SpellDraupnirsCallProjL);
    lData.desiredTarget = a_target;
    RE::Projectile::Launch(&a_pHandle, lData);
    auto explosionDamage = projectile ? projectile->GetProjectileRuntimeData().weaponDamage : damage;
    explosionDamage *= explosionMagnitude;
    FenixUtils::stagger(std::clamp(explosionDamage / 10.f, 0.1f, 100.f), a_target, GetThrowerActor());
    if (auto targetAVO = a_target->AsActorValueOwner(); targetAVO)
        targetAVO->RestoreActorValue(RE::ActorValue::kHealth, -explosionDamage);
}

void Draupnir::ReplaceStickedProjectileModel(RE::Projectile* a_proj)
{
    if (a_proj && a_proj->Get3D() && projectileModel) {
        auto projModel = a_proj->Get3D();
        auto projNode = projModel ? projModel->AsFadeNode() : nullptr;
        auto copyModelObj = projectileModel->Clone();
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

            if (replacedProjectileModel && replacedProjectileModel->parent) {
                replacedProjectileModel->parent->DetachChild(replacedProjectileModel.get());
                replacedProjectileModel.reset();
            }
        } else spdlog::warn("projectile node is null");
    } else spdlog::warn("projectile or projectile model is null");
}

void Draupnir::PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) 
{
    auto missileProjectile = projectile ? projectile->As<RE::MissileProjectile>() : nullptr;
    if (missileProjectile && a_impactData) {
        const bool isTargetActor = a_target ? a_target->formType == RE::FormType::ActorCharacter : false;
        const auto victim = isTargetActor ? a_target->As<RE::Actor>() : nullptr;
        auto& rtData = missileProjectile->GetProjectileRuntimeData();
        auto& missileRTD = missileProjectile->GetMissileRuntimeData();
        auto shooter = rtData.shooter ? rtData.shooter.get()->As<RE::Actor>() : nullptr;
        const auto projBase = missileProjectile->GetProjectileBase();
        bool isDraupnirSpear = throwerParent->GetRightHandRelic() == this;

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
