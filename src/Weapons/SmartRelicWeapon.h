#pragma once

#include "ThrowableRelicWeapon.h"
#include "Weapons/States/RelicWeaponState.h"

class Caller;
class ActorCapability;

template <typename Handle>
class RecentHitList
{
public:
    void Push(Handle a_handle)
    {
        std::lock_guard lock(_mutex);
        if (std::find(_list.begin(), _list.end(), a_handle) != _list.end())
            return;
        _list.insert(_list.begin(), a_handle);
        if (_list.size() > 4) _list.pop_back();
    }

    [[nodiscard]] bool Contains(Handle a_handle) const
    {
        std::lock_guard lock(_mutex);
        return std::find(_list.begin(), _list.end(), a_handle) != _list.end();
    }

    [[nodiscard]] std::vector<Handle> Snapshot() const
    {
        std::lock_guard lock(_mutex);
        return _list;
    }

    void clear()
    {
        std::lock_guard lock(_mutex);
        _list.clear();
    }

    [[nodiscard]] std::size_t Size() const
    {
        std::lock_guard lock(_mutex);
        return _list.size();
    }

private:
    mutable std::mutex  _mutex;
    std::vector<Handle> _list;
};

class SmartRelicWeapon : public ThrowableRelicWeapon
{
public:
    static constexpr RelicAbility kRequiredAbility = RelicAbility::kSmart;

    ~SmartRelicWeapon() override = default;
    explicit SmartRelicWeapon(RE::TESBoundObject* a_object);

    Caller* caller = nullptr;
    BaseActor* callerParent = nullptr;

    bool Initialize() override;
    void Update() override;
    void UpdateProjectile(RE::Projectile* a_projectile, float a_delta) override;
    bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) override;
    void PreImpact(RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void OnTrailDelete() override;

    [[nodiscard]] Caller*           GetCaller() const {return caller;}
    [[nodiscard]] RelicWeaponState* GetState() const {return currentState.get();}
    [[nodiscard]] ActorCapability*  GetActiveController() const noexcept;
    [[nodiscard]] const RecentHitList<RE::ActorHandle>&     GetLastHitActors() const noexcept {return lastHitActors;}
    [[nodiscard]] const RecentHitList<RE::ObjectRefHandle>& GetLastHitForms() const noexcept {return lastHitForms;}

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

    RecentHitList<RE::ActorHandle>     lastHitActors;
    RecentHitList<RE::ObjectRefHandle> lastHitForms;

    RE::NiPointer<RE::NiNode> stuckedBone;
    RE::ActorHandle           stuckedActor;

    RE::BGSProjectile* ArrivingWeaponDummyProjectile = nullptr;
    RE::TESAmmo*       ArrivingWeaponDummyAmmo = nullptr;

    AsyncUtil::GameTime delayedCallUpdate;
};

using ThrowState = SmartRelicWeapon::ThrowState;
