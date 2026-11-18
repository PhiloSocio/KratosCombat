#pragma once

#include "Weapons/SmartRelicWeapon.h"

class Draupnir : public SmartRelicWeapon
{
public:
    ~Draupnir() override;
    explicit Draupnir(RE::TESBoundObject* a_object);

    bool Initialize() override;
    void Update() override;
    void OnEquip(BaseActor* a_actor) override;
    void SetState(RelicWeaponState::Type a_type) override {};
    bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) override;
    void PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void OnMenuOpenCloseEvent(const bool a_opening) override;

    RelicType GetType() const override {return RelicType::kDraupnir;}
    bool PreThrow(RE::Projectile::LaunchData& a_launchData) override;
    void PostThrow() override;
    bool Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u) override;
    bool ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false) override {return false; };

private:
    RE::BGSProjectile* DraupnirsCallProjBase  = nullptr;
    RE::SpellItem*     SpellDraupnirsCallProj = nullptr;

    RE::ProjectileHandle      DraupnirSpearProjectiles[9];    //  they will be explode after Draupnir's Call move
    RE::ProjectileHandle      DraupnirsCallProjectiles[9];    //  hard to explain
    RE::NiPointer<RE::NiNode> DraupnirSpearHitBones[9];       //  keeps attached nodes of throwed draupnirs' for forced detonation*
    RE::ActorHandle           DraupnirSpearHitActors[9];      //  keeps hitten actors


    RE::BGSExplosion* StuckedDraupnir   = nullptr;
    RE::BGSExplosion* DraupnirExplosion = nullptr;
    RE::BGSArtObject* DraupnirStuckedFX = nullptr;

    float explosionMagnitude   = 1.f;
    float nextExplosionTime    = 0.f;
    float timeToDoneExplosions = 0.f;
    float explosionDelay       = 0.f;
    size_t currentHitIndex     = 0;
    bool explosionsStarted     = false;
    std::vector<RE::ProjectileHandle> MeleeHitProjectiles;
    RE::BSFixedString DefaultDraupnirModel;
    std::vector<std::tuple<RE::NiPointer<RE::NiNode>, RE::ActorHandle, RE::ProjectileHandle>> spearHits;

    void MeleeThrow();
    void StartExplosions(const float a_delay);
    void ArtilleryOfTheAncients(const float a_delay, const float a_duration);

    void ReplaceStickedProjectileModel(RE::Projectile* a_proj);
    void RainOfDraupnir();
    void AddSpearHit(RE::NiNode* a_bone, RE::Actor* a_actor = nullptr);
    void AddSpearHit(RE::Projectile* a_proj);
    void TriggerExplosions(float a_delay, float a_force);
    void TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::Actor* a_target);
    void TriggerExplosionAtLocation(RE::Projectile* a_proj);

    float nextLaunchTime  = 0.f;
    float nextLaunchDelay = 0.f;
    float lastLaunchTime  = 0.f;
    bool  draupnirRainStarted = false;

};
