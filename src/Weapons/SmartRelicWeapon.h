#pragma once

#include "ThrowableRelicWeapon.h"
#include "Weapons/States/RelicWeaponState.h"

class Caller;

class SmartRelicWeapon : public ThrowableRelicWeapon
{
public:
    ~SmartRelicWeapon() override = default;
    explicit SmartRelicWeapon(RE::TESBoundObject* a_object);

    Caller* caller = nullptr;
    BaseActor* callerParent = nullptr;

    std::vector<RE::ActorHandle>    lastHitActors;
    std::vector<RE::TESObjectREFR*> lastHitForms;
    RE::NiPointer<RE::NiNode> stuckedBone;
    RE::NiPointer<RE::Actor> stuckedActor;

    bool Initialize() override;
    void Update() override;
    void UpdateProjectile(RE::Projectile* a_projectile) override;
    bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) override;
    void PreImpact(RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void OnTrailDelete() override;

    [[nodiscard]] Caller* GetCaller() const {return caller;}
    [[nodiscard]] RelicWeaponState* GetState() const {return currentState.get();}

    virtual void SetState(RelicWeaponState::Type a_type) = 0;
    virtual void SetState(std::unique_ptr<RelicWeaponState> a_state);
    RelicWeaponState* GetCurrentState() const {return currentState.get();}
    void GetPosition(RE::NiPoint3& a_point);
    virtual bool Call(Caller* a_caller, const bool a_justDestroy = false, std::optional<float> a_delay = std::nullopt);
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

};

using ThrowState = SmartRelicWeapon::ThrowState;
