#pragma once

#include "ThrowableRelicWeapon.h"
#include "Actors/Caller.h"
#include "Weapons/States/RelicWeaponState.h"

class SmartRelicWeapon : public ThrowableRelicWeapon
{
public:
    ~SmartRelicWeapon() override = default;
    explicit SmartRelicWeapon(RE::TESBoundObject* a_object);

    enum class ProjectileState : std::uint8_t {
        kNone,
        kLaunched,
        kStucked,
        kHavok,
        kHoming
    };
    enum class ThrowState : std::uint8_t {
        kNone,
        kThrowable,
        kThrown,
        kCanArrive,
        kArriving,
        kArrived
    };

    struct RuntimeData : public ThrowableRelicWeapon::RuntimeData {
        Caller* caller = nullptr;

        RE::NiPoint3 position        = {0.f, 0.f, 0.f};
        RE::NiTransform transformPW  = RE::NiTransform();
        RE::NiTransform transformPL  = RE::NiTransform();
        RE::NiTransform transformW   = RE::NiTransform();
        RE::NiTransform transformL   = RE::NiTransform();

        std::vector<RE::ActorHandle>    lastHitActors;
        std::vector<RE::TESObjectREFR*> lastHitForms;
        ProjectileState projState = ProjectileState::kNone;
    };
    RuntimeData smartWeaponRuntimeData;

    RE::Projectile* ArrivingWeaponProjectile = nullptr;
    RE::BGSProjectile* ArrivingWeaponProjectileBase = nullptr;

    bool Initialize() override;
    void Update() override;
    bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) override;
    void OnImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;

    [[nodiscard]] RuntimeData& GetSmartWeaponRuntimeData() {return smartWeaponRuntimeData;}
    [[nodiscard]] Caller* GetCaller() {return smartWeaponRuntimeData.caller && smartWeaponRuntimeData.caller->IsValid() ? smartWeaponRuntimeData.caller : nullptr;}
    [[nodiscard]] RelicWeaponState* GetState() const {return currentState.get();}

    void SetThrowState(const ThrowState a_throwState);
    virtual ThrowState GetThrowState() const {return _throwState;}
    virtual void SetState(RelicWeaponState::Type a_type) = 0;
    virtual void SetState(std::unique_ptr<RelicWeaponState> a_state);
    virtual void GetPosition(RE::NiPoint3& a_point);
    virtual void Call(const bool a_justDestroy = false, std::optional<float> a_delay = std::nullopt);
    virtual void Catch(bool a_justDestroy = false);
    virtual bool IsArriving() const = 0;
    virtual bool IsHoming() const = 0;
    virtual RE::NiTransform GetWorldTransform();
    virtual RE::NiTransform GetLocalTransform();

protected:
    std::unique_ptr<RelicWeaponState> currentState;

    AsyncUtil::GameTime projectileUpdate;
    AsyncUtil::GameTime trailUpdate;
    AsyncUtil::GameTime trailRemoveUpdate;

private:
    ThrowState _throwState = ThrowState::kNone;
};

using ProjectileState = SmartRelicWeapon::ProjectileState;
using ThrowState = SmartRelicWeapon::ThrowState;
