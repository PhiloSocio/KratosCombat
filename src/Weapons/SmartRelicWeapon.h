#pragma once

#include "ThrowableRelicWeapon.h"
#include "Actors/Caller.h"
#include "Weapons/States/RelicWeaponState.h"

class SmartRelicWeapon : public ThrowableRelicWeapon
{
public:
    ~SmartRelicWeapon() override = default;
    explicit SmartRelicWeapon(RE::TESBoundObject* a_object);

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

        std::vector<RE::ActorHandle>    lastHitActors;
        std::vector<RE::TESObjectREFR*> lastHitForms;
        RE::NiPointer<RE::NiNode> stuckedBone;
        RE::NiPointer<RE::Actor> stuckedActor;
    };
    RuntimeData runtimeData;

    bool Initialize() override;
    void Update() override;
    bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) override;
    void PreImpact(RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void OnTrailDelete() override;

    [[nodiscard]] RuntimeData& GetruntimeData() {return runtimeData;}
    [[nodiscard]] Caller* GetCaller() {return runtimeData.caller && runtimeData.caller->IsValid() ? runtimeData.caller : nullptr;}
    [[nodiscard]] RelicWeaponState* GetState() const {return currentState.get();}

    void SetThrowState(const ThrowState a_throwState) {_throwState = a_throwState;};
    virtual void SetState(RelicWeaponState::Type a_type) = 0;
    virtual void SetState(std::unique_ptr<RelicWeaponState> a_state);
    RelicWeaponState* GetCurrentState() const {return currentState.get();}
    ThrowState GetThrowState() const {return _throwState;}
    void GetPosition(RE::NiPoint3& a_point);
    virtual void Call(const bool a_justDestroy = false, std::optional<float> a_delay = std::nullopt);
    virtual void Catch(bool a_justDestroy = false);
    virtual bool IsArriving() const {return currentState ? currentState->GetType() == RelicWeaponState::Type::kArriving : false;}
    virtual bool IsHoming() const {return currentState ? currentState->GetType() == RelicWeaponState::Type::kHoming : false;}

protected:
    std::unique_ptr<RelicWeaponState> currentState;

    RE::BGSProjectile* ArrivingWeaponDummyProjectile = nullptr;
    RE::TESAmmo*       ArrivingWeaponDummyAmmo = nullptr;

    AsyncUtil::GameTime projectileUpdate;
    AsyncUtil::GameTime trailUpdate;
    AsyncUtil::GameTime trailRemoveUpdate;

private:
    ThrowState _throwState = ThrowState::kNone;
};

using ThrowState = SmartRelicWeapon::ThrowState;
