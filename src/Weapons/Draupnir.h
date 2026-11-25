#pragma once

#include "Weapons/ThrowableRelicWeapon.h"

class Draupnir : public ThrowableRelicWeapon
{
public:
    explicit Draupnir(RE::TESBoundObject* a_object);
    ~Draupnir() override;

    bool Initialize() override;
    void Update() override;
    bool OnHit(ThrownWeapon& a_weapon, RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector) override;
    void PostImpact(ThrownWeapon& a_weapon, RE::ArrowProjectile* a_this, RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;

    RelicType GetType() const override {return RelicType::kDraupnir;}
    bool PreThrow(ThrownWeapon& a_weapon, RE::Projectile::LaunchData& a_launchData) override;
    void PostThrow(ThrownWeapon& a_weapon) override;
    bool Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u) override;
    bool ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false) override {return false; };

private:
    RE::BGSProjectile* DraupnirsCallProjBase  = nullptr;
    RE::SpellItem*     SpellDraupnirsCallProj = nullptr;

    RE::BGSExplosion* DraupnirExplosion = nullptr;

    float explosionMagnitude   = 1.f;
    float nextExplosionTime    = 0.f;
    float timeToDoneExplosions = 0.f;
    float explosionDelay       = 0.f;
    size_t currentHitIndex     = 0;
    bool explosionsStarted     = false;
    using SpearHits = std::vector<std::tuple<RE::NiPointer<RE::NiNode>, RE::ActorHandle, RE::ProjectileHandle>>;
    SpearHits spearHits;
    void StartExplosions(const float a_delay);
    void ArtilleryOfTheAncients(const float a_delay, const float a_duration);

    void RainOfDraupnir();
    void AddSpearHit(RE::NiNode* a_bone, RE::Actor* a_actor = nullptr);
    void AddSpearHit(const RE::ProjectileHandle& a_handle);
    void TriggerExplosions(float a_delay, float a_force);
    void TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::Actor* a_target);
    void TriggerExplosionAtLocation(RE::Projectile* a_proj);

    float lastLaunchTime = 0.f;
    bool  draupnirRainStarted = false;

};
