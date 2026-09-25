#pragma once
#include "util.h"
#include "Assets.h"
#include "settings.h"
#include "SmartRelicWeapon.h"

class Draupnir : public SmartRelicWeapon
{
public:
    ~Draupnir() override = default;
    explicit Draupnir(RE::TESBoundObject* a_object);

    struct RuntimeData : public SmartRelicWeapon::RuntimeData {
    };

    RuntimeData runtimeData;

    bool Initialize() override;
    void Update() override;
    void SetState(RelicWeaponState::Type a_type) override;
    bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) override;
    void PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void OnMenuOpenCloseEvent(const bool a_opening) override;

    void GetPosition(RE::NiPoint3& a_point) override;
    RelicType GetType() const override {return RelicType::kLeviathanAxe;}
//    void Throw(const bool isVertical, const bool justContinue = false, const bool isHoming = false) override;
    bool PreThrow() override;
    void PostThrow() override;
    void Call(const bool a_justDestroy = false, std::optional<float> a_delay = std::nullopt) override;
    void Catch(bool a_justDestroy = false) override;
    void Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u) override;
    void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false) override;
//    void SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical) override;
//    void SetHitRotation(RE::NiPoint3& a_angles, const RE::NiPoint3& a_direction, const bool a_vertical) override;
//    void TweakHitPosition(RE::NiPoint3& a_position, const RE::NiPoint3& a_direction, const float a_offset, const bool a_vertical) override;
private:
    AsyncUtil::GameTime projectileUpdate;
    AsyncUtil::GameTime trailUpdate;

    RE::BGSProjectile* DraupnirsCallProjBase = nullptr;

    static inline RE::NiPointer<RE::Projectile>   DraupnirSpearProjectiles[9];                            //  they will be explode after Draupnir's Call move
    static inline RE::NiPointer<RE::Projectile>   DraupnirsCallProjectiles[9];                            //  hard to explain
    static inline RE::NiPointer<RE::NiNode>       DraupnirSpearHitBones[9];                               //  keeps attached nodes of throwed draupnirs for forced detonation*
    static inline RE::NiPointer<RE::Actor>        DraupnirSpearHitActors[9];                              //  keeps hitten actors
    static inline RE::NiPointer<RE::Projectile>   DraupnirSpearProjectileL;

    static inline RE::SpellItem*        SpellDraupnirProjL          = nullptr;
    static inline RE::SpellItem*        SpellDraupnirsCallProjL     = nullptr;
    static inline RE::BGSProjectile*    DraupnirSpearProjBaseL      = nullptr;
    static inline RE::BGSProjectile*    DraupnirsCallProjBaseL      = nullptr;

    static inline RE::BGSExplosion* StuckedDraupnir     = nullptr;
    static inline RE::BGSExplosion* DraupnirExplosion   = nullptr;
    static inline RE::BGSArtObject* DraupnirStuckedFX   = nullptr;

    float explosionMagnitude = 1.f;
    float nextExplosionTime = 0.f;
    float timeToDoneExplosions = 0.f;
    float explosionDelay = 0.f;
    size_t currentHitIndex = 0;
    bool explosionsStarted = false;
    std::vector<RE::FormID> MeleeHitProjectileIDs;
    RE::BSFixedString DefaultDraupnirModel;
    std::vector<std::tuple<RE::NiPointer<RE::NiNode>, RE::NiPointer<RE::Actor>, RE::NiPointer<RE::Projectile>>> spearHits;

    void MeleeThrow();
    void Call(const float a_damage, const float a_force);
    void StartExplosions(const float a_delay);
    void SetExplosionMagnitude(const float a_magnitude) {explosionMagnitude = a_magnitude;}
    void ArtilleryOfTheAncients(const float a_delay, const float a_duration);

    void ReplaceStickedProjectileModel(RE::Projectile* a_proj);
    void RainOfDraupnir();
    void AddSpearHit(RE::NiNode* a_bone, RE::Actor* a_actor = nullptr);
    void AddSpearHit(RE::Projectile* a_proj);
    void TriggerExplosions(float a_delay, float a_force, RE::ProjectileHandle* a_pHandle);
    void TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::ProjectileHandle* a_pHandle, RE::Actor* a_target);
    void TriggerExplosionAtLocation(RE::Projectile* a_proj, RE::ProjectileHandle* a_pHandle);

    float nextLaunchTime = 0.0f;
    float nextLaunchDelay = 0.0f;
    float lastLaunchTime = 0.0f;
    bool draupnirRainStarted = false;
};

