#include "Draupnir.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Thrower.h"
#include "Assets.h"
#include "settings.h"
#include "util.h"

Draupnir::Draupnir(RE::TESBoundObject* a_object)
    : ThrowableRelicWeapon(a_object)
{
    spearHits.reserve(10);
    Initialize();
}
Draupnir::~Draupnir()
{
    spdlog::debug("Draupnir destructed");
}

bool Draupnir::Initialize()
{
    bool found = true;
    auto dataHandler = RE::TESDataHandler::GetSingleton();
    DraupnirsCallProjBase   = dataHandler->LookupForm<RE::BGSProjectile>(0x818, Config::DraupnirModESP);
    SpellDraupnirsCallProj  = dataHandler->LookupForm<RE::SpellItem>    (0x805, Config::DraupnirModESP);
    DraupnirExplosion       = dataHandler->LookupForm<RE::BGSExplosion> (0x809, Config::DraupnirModESP);
    if (DraupnirsCallProjBase)
        spdlog::debug("Draupnir's call projectile is {}", DraupnirsCallProjBase->GetName());
    else {spdlog::error("Can't find Draupnir Spear projectile"); found = false;}
    if (SpellDraupnirsCallProj)
        spdlog::debug("Draupnir's call spell is {}", SpellDraupnirsCallProj->GetName());
    else {spdlog::error("Can't find Draupnir Spear projectile spell"); found = false;}
    if (DraupnirExplosion)
        spdlog::debug("Draupnir Spear explosion is {}", DraupnirExplosion->GetName());
    else {spdlog::error("Can't find Draupnir Spear explosion"); found = false;}

    if (ThrowableWeaponDummyProjectile) {
        ThrowableWeaponDummyProjectile->data.flags.reset(pbFlag::kCanPickUp);
        ThrowableWeaponDummyProjectile->data.gravity = 2.34f;
    } else
        found = false;

    const auto assets = Assets::GetSingleton();
    if (!assets) return false;
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
    ThrowableRelicWeapon::Update();
    if (explosionsStarted) {
        if (AsyncUtil::GameTime::GetEngineTime() >= nextExplosionTime) {
            TriggerExplosions(explosionDelay, 15.f);
        }
    }
}

bool Draupnir::PreThrow(ThrownWeapon& a_weapon, RE::Projectile::LaunchData& a_launchData)
{
    a_weapon.ThrowableWeaponDummyProjectile->data.speed *= Config::LeviThrowSpeedMult;
    a_launchData.ammoSource = nullptr;
    return true;
}
void Draupnir::PostThrow(ThrownWeapon& a_weapon)
{
    auto throwerActor = a_weapon.GetThrowerActor();
    if (!throwerActor) {spdlog::error("Draupnir::Throw - thrower actor is null"); return;}

    a_weapon.gravity = 2.34f;
    a_weapon.gravity /= std::max(std::powf(a_weapon.throwingChargeDuration + 1.f, 3.f), 0.01f);
    a_weapon.gravity = std::max(a_weapon.gravity, 0.5f);
    a_weapon.trailData.trailOverride.baseColorOverride = a_weapon.trailData.GetColorByIndex(Config::TrailColorIndexLeviathan);

    if (a_weapon.throwingType == ThrowType::kArtilleryThrow && !draupnirRainStarted)
        ArtilleryOfTheAncients(Config::DraupnirRainInterval, Config::DraupnirRainDuration);
}

void Draupnir::ArtilleryOfTheAncients(const float a_delay, const float a_duration)
{
    const auto endTime = AsyncUtil::GameTime::GetEngineTime() + a_duration;
    lastLaunchTime = endTime;
    draupnirRainStarted = true;
    spdlog::info("rain of draupnir triggered");
    const auto taskID = AsyncUtil::TaskManager::GetSingleton().AddPeriodicTask(a_duration, a_delay, [this, a_delay, endTime]() {
        if (!draupnirRainStarted || lastLaunchTime != endTime) return;

        const auto currentTime = AsyncUtil::GameTime::GetEngineTime();
        if (currentTime > endTime) {
            draupnirRainStarted = false;
            spdlog::info("rain of draupnir done");
            return;
        }

        RainOfDraupnir();
        if (draupnirRainStarted && currentTime + a_delay > endTime) {
            draupnirRainStarted = false;
            spdlog::info("rain of draupnir done");
        }
    });
    if (!taskID)
        draupnirRainStarted = false;
}
void Draupnir::RainOfDraupnir()
{
    auto wielder = GetWielder();
    if (!wielder || !wielder->IsValid()) {
        draupnirRainStarted = false;
        return;
    }

    auto throwerActor = wielder->GetActor();
    if (!throwerActor) {
        draupnirRainStarted = false;
        return;
    }

    const auto nearCombatTargets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(throwerActor, 666.f);
    const bool hasNearTarget = !nearCombatTargets.empty() && nearCombatTargets.front();
    auto origin = hasNearTarget
        ? nearCombatTargets.front()->GetPosition()
        : throwerActor->GetPosition() + MathUtil::Algebra::GetForwardVector(throwerActor) * 360.f;
    origin += RE::NiPoint3(
        MathUtil::Algebra::GenerateRandomFloat(-100.f, 100.f),
        MathUtil::Algebra::GenerateRandomFloat(-100.f, 100.f),
        2000.f);

    const RE::Projectile::ProjectileRot rotation(PI2 + MathUtil::Algebra::GenerateRandomFloat(-0.018f, 0.018f), 0.f);
    Throw(wielder->GetCapabilityAs<Thrower>(), RotationType::kNone, rotation, origin, ThrowType::kArtilleryThrow);
}

bool Draupnir::Charge(const uint8_t a_chargeHitCount, const float a_magnitude, const uint8_t a_stage, const uint8_t a_coolDown)
{
    switch (a_stage) {
    case 1u:
        explosionMagnitude = 1.2f;
        break;
    case 2u:
        explosionMagnitude = 1.5f;
        break;
    case 3u:
        if (DraupnirsCallProjBase && DraupnirExplosion) {
            DraupnirExplosion->data.force = 100.f * explosionMagnitude;
            StartExplosions(Config::DraupnirExplosionsInterval);
        }
        break;
    }
    return _isCharged;
}
void Draupnir::AddSpearHit(RE::NiNode* bone, RE::Actor* actor)
{
    if (bone) {
        const auto maxSpearHits = std::max<std::size_t>(Config::DraupnirSpearCount, 1u);
        if (spearHits.size() >= maxSpearHits) {
            auto& [oldBone, oldTarget, oldProjectileHandle] = spearHits.front();
            if (const auto oldProjectile = oldProjectileHandle.get().get(); oldProjectile && !oldProjectile->IsDestroyed())
                oldProjectile->Kill();
            spearHits.erase(spearHits.begin());
        }   spearHits.emplace_back(bone, actor, RE::ProjectileHandle());
    }
}
void Draupnir::AddSpearHit(const RE::ProjectileHandle& a_handle)
{
    if (a_handle.get().get()) {
        const auto maxSpearHits = std::max<std::size_t>(Config::DraupnirSpearCount, 1u);
        if (spearHits.size() >= maxSpearHits) {
            auto& [oldBone, oldTarget, oldProjectileHandle] = spearHits.front();
            if (const auto oldProjectile = oldProjectileHandle.get().get(); oldProjectile)
                oldProjectile->Kill();
            spearHits.erase(spearHits.begin());
        }   spearHits.emplace_back(nullptr, RE::ActorHandle(), a_handle);
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
void Draupnir::TriggerExplosions(float a_delay, float a_force)
{
    if (spearHits.empty()) {
        if (AsyncUtil::GameTime::GetEngineTime() > timeToDoneExplosions) {
            explosionsStarted = false;
            currentHitIndex = 0;
            explosionMagnitude = 1.f;
            spdlog::info("draupnir explosion chain finished");
        } //spdlog::info("all draupnir explosions done");
    } else {
        auto& [bone, target, projectileHandle] = spearHits.front();
        auto targetActor = target.get().get();
        currentHitIndex++;
        if (bone && targetActor) {
            TriggerExplosionAtLocation(bone.get(), targetActor);
            targetActor->RemoveExtraArrows3D();
            spdlog::info("{}. draupnir explosion done", currentHitIndex);
        } else if (auto proj = projectileHandle.get().get(); proj && !proj->IsDestroyed()) {
            TriggerExplosionAtLocation(proj);
            spdlog::info("{}. draupnir explosion done", currentHitIndex);
        } else spdlog::info("{}. draupnir explosion is empty!", currentHitIndex);
        spearHits.erase(spearHits.begin());
        nextExplosionTime = AsyncUtil::GameTime::GetEngineTime() + a_delay;
        timeToDoneExplosions = nextExplosionTime + 0.3f;
    }
}
void Draupnir::TriggerExplosionAtLocation(RE::Projectile* a_proj)
{
    if (!a_proj) return;
    RE::ProjectileHandle pHandle;
    auto throwerActor = GetWielder() ? GetWielder()->GetActor() : GetThrowerActor();
    if (!throwerActor || !SpellDraupnirsCallProj) return;
    RE::Projectile::LaunchData lData(throwerActor, a_proj->data.location, RE::Projectile::ProjectileRot(), SpellDraupnirsCallProj);
    auto launchResult = RE::Projectile::Launch(&pHandle, lData);
    auto explosionProjectile = launchResult ? launchResult->get().get() : nullptr;
    if (explosionProjectile)
        RegisterAuxiliaryProjectile(explosionProjectile, pHandle);
    a_proj->Kill();
}
void Draupnir::TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::Actor* a_target)
{
    RE::ProjectileHandle pHandle;
    auto throwerActor = GetWielder() ? GetWielder()->GetActor() : GetThrowerActor();
    if (!throwerActor || !a_bone || !SpellDraupnirsCallProj) return;
    RE::Projectile::LaunchData lData(throwerActor, a_bone->world.translate, RE::Projectile::ProjectileRot(), SpellDraupnirsCallProj);
    lData.desiredTarget = a_target;
    auto launchResult = RE::Projectile::Launch(&pHandle, lData);
    auto explosionProjectile = launchResult ? launchResult->get().get() : nullptr;
    if (explosionProjectile)
        RegisterAuxiliaryProjectile(explosionProjectile, pHandle);
    auto explosionDamage = explosionProjectile ? explosionProjectile->GetProjectileRuntimeData().weaponDamage : damage;
    explosionDamage *= explosionMagnitude;
    if (a_target)
        FenixUtils::stagger(std::clamp(explosionDamage / 10.f, 0.1f, 100.f), a_target, throwerActor);
    if (auto targetAVO = a_target ? a_target->AsActorValueOwner() : nullptr)
        targetAVO->RestoreActorValue(RE::ActorValue::kHealth, -explosionDamage);
}
bool Draupnir::OnHit(ThrownWeapon& a_weapon, RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
    if (!a_this || a_weapon.projectile != a_this || !a_AllCdPointCollector) return false;

    auto& rtData = a_this->GetProjectileRuntimeData();
    if (rtData.livingTime <= 1.69f * *g_deltaTime) return true;

    RE::Actor* shooter = nullptr;
    if (auto shooterRef = rtData.shooter.get()) shooter = shooterRef->As<RE::Actor>();

    if (a_this->GetProjectileBase() == DraupnirsCallProjBase) {
        spdlog::debug("draupnir's call projectile");
        return true;
    }

    for (auto& point : a_AllCdPointCollector->hits) {
    //    const auto ourProj  = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableA);
        const auto target   = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

        const auto victim = target ? target->As<RE::Actor>() : nullptr;
        if (target && a_this != target->AsProjectile()) {
            if (victim && shooter && victim != shooter) {
                if (!victim->IsDead()) {
                    if (a_weapon.weap && a_weapon.ench && !a_weapon.ench->effects.empty() && a_weapon.ench->effects[0])
                        ObjectUtil::Enchantment::ChargeInventoryWeapon(shooter, a_weapon.weap, -a_weapon.ench->effects[0]->effectItem.magnitude);

                    if (a_weapon.throwerParent) {
                        shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);

                        a_weapon.throwerParent->RestoreRage(Config::RageBuffRate->value * 0.25f, true);
                    }
                }
            }

            a_weapon.MarkHitTransformApplied();
            break;
        }
    }

    return false;
}
void Draupnir::PostImpact(ThrownWeapon& a_weapon, RE::ArrowProjectile* a_this, RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable)
{
    if (!a_this || a_weapon.projectile != a_this) return;

    if (a_impactData) {
        const bool isTargetActor = a_target ? a_target->formType == RE::FormType::ActorCharacter : false;
        const auto victim = isTargetActor ? a_target->As<RE::Actor>() : nullptr;
        auto& missileRTD = a_this->GetMissileRuntimeData();
        const auto projBase = a_this->GetProjectileBase();

        if (projBase == DraupnirsCallProjBase) {
            missileRTD.impactResult = RE::ImpactResult::kImpale;
            a_impactData->impactResult = RE::ImpactResult::kImpale;
        } else {
            missileRTD.impactResult = RE::ImpactResult::kStick;
            a_impactData->impactResult = RE::ImpactResult::kStick;
            a_weapon.projState = ProjectileState::kStucked;
            const bool itWillStick = (missileRTD.impactResult == RE::ImpactResult::kStick);
            spdlog::debug("draupnir is will {}", itWillStick ? "stick" : "bounce");
            if (victim && a_impactData->damageRootNode && itWillStick) {
                AddSpearHit(a_impactData->damageRootNode, victim);
                spdlog::debug("spear stucked to {} of {}!", a_impactData->damageRootNode->name, victim->GetName());
            } else {
                AddSpearHit(a_weapon.projectileHandle);
                spdlog::debug("draupnir's target is not an actor.");
            }
        }
    }
}
