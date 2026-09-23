#pragma once
#include "util.h"
#include "settings.h"
#include "SmartRelicWeapon.h"
#include "HomingState.h"
#include "ArrivingState.h"
#include "SoundManager.h"

class Mjolnir : public SmartRelicWeapon
{
public:
    static Mjolnir* GetSingleton();

    struct Data : public SmartRelicWeapon::Data  {
        RE::NiPoint3 lastEulerAngles= {0.f, 0.f, 0.f};
        bool isPenetrating = false;
    };

    Data data;
    TrailData trailData;
    SoundManager soundData{
        SoundDefinition{
            .throwingStart = nullptr,
            .throwingLoop0 = nullptr,
            .throwingLoop1 = nullptr,
            .callStart = nullptr,
            .arrivingStart = nullptr,
            .arrivingLoop0 = nullptr,
            .arrivingLoop1 = nullptr,
            .arrivingLoop2 = nullptr,
            .arrivingNear = nullptr,
            .catching = nullptr
        },
        data
    };

    bool Initialize() override;
    void Update(const float a_delta) override;

    void GetPosition(RE::NiPoint3& a_point, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton()) override;
    RelicType GetType() const override {return RelicType::kMjolnir;}
    void Throw(const bool justContinue, const bool isVertical = false, const bool isHoming = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton()) override;
    void Call(const bool a_justDestroy = false, const bool a_justContinue = false, std::optional<float> a_delay = std::nullopt, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton()) override;
    void Catch(bool a_justDestroy = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton()) override;
    void Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u) override;
    void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false) override;
    bool IsArriving() const override;
    bool IsHoming() const override;
    void StartChargingThrow(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton()) override;
    void AddProjectileTrail(const float a_delta) override;
    void FadeProjectileTrail(const float a_delta) override;
    void DeleteProjectileTrail() override;
    RE::NiTransform GetWorldTransform() override;
    RE::NiTransform GetLocalTransform() override;

    bool isMjolnirCalled;
    bool isMjolnirArriving;
    bool isMjolnirThrowed;

    AsyncUtil::GameTime projectileUpdate;
    AsyncUtil::GameTime trailUpdate;
    AsyncUtil::GameTime trailRemoveUpdate;
    AsyncUtil::GameTime callUpdate;
private:
friend class WeaponIdentify;
friend class AnimationEventTracker;
friend class ProjectileHook;

    RE::Projectile* MjolnirProjectileT      = nullptr;
    RE::Projectile* MjolnirProjectileA      = nullptr;
    RE::Projectile* LastMjolnirProjectile   = nullptr;

    RE::BGSProjectile* MjolnirProjBaseT = nullptr;
    RE::BGSProjectile* MjolnirProjBaseA = nullptr;

    RE::SpellItem* SpellMjolnirProjT    = nullptr;
    RE::SpellItem* SpellMjolnirProjA    = nullptr;
    RE::SpellItem* SpellCatchMjolnir    = nullptr;
    RE::SpellItem* SpellChargeCD        = nullptr;

    RE::EffectSetting* EffCatchMjolnir  = nullptr;

    Mjolnir()  = default;
    ~Mjolnir() = default;
};
