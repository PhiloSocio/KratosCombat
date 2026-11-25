#pragma once

#include "ThrownWeapon.h"
#include "Weapons/States/RelicWeaponState.h"

class Caller;
class ActorCapability;
class SmartRelicWeapon;

template <typename Handle>
class RecentHitList
{
public:
    void Push(Handle a_handle)
    {
        std::lock_guard lock(_mutex);
        if (std::find(_list.begin(), _list.begin() + _size, a_handle) != _list.begin() + _size) return;
        const auto newSize = std::min(_size + 1, _list.size());
        for (auto i = newSize - 1; i > 0; --i)
            _list[i] = _list[i - 1];
        _list[0] = a_handle;
        _size = newSize;
    }

    [[nodiscard]] bool Contains(Handle a_handle) const
    {
        std::lock_guard lock(_mutex);
        return std::find(_list.begin(), _list.begin() + _size, a_handle) != _list.begin() + _size;
    }

    [[nodiscard]] std::vector<Handle> Snapshot() const
    {
        std::lock_guard lock(_mutex);
        return {_list.begin(), _list.begin() + _size};
    }

    void clear()
    {
        std::lock_guard lock(_mutex);
        _size = 0;
    }

    [[nodiscard]] std::size_t Size() const
    {
        std::lock_guard lock(_mutex);
        return _size;
    }

private:
    mutable std::mutex _mutex;
    std::array<Handle, 4> _list{};
    std::size_t _size = 0;
};

class ThrownSmartWeapon : public ThrownWeapon
{
public:
    explicit ThrownSmartWeapon(SmartRelicWeapon& a_weapon);
    ~ThrownSmartWeapon() override;

    static constexpr RelicAbility kType = RelicAbility::kSmart;

    void SetState(std::unique_ptr<RelicWeaponState> a_state);
    void SetState(RelicWeaponState::Type a_type);

    bool Call(Caller* a_caller, bool a_justDestroy = false, std::optional<float> a_delay = std::nullopt);
    void Catch(bool a_justDestroy = false);

    [[nodiscard]] RelicAbility GetType() const override { return RelicAbility::kSmart; }
    [[nodiscard]] bool IsArriving() const noexcept { return currentState && currentState->GetType() == RelicWeaponState::Type::kArriving; }
    [[nodiscard]] bool IsHoming() const noexcept { return currentState && currentState->GetType() == RelicWeaponState::Type::kHoming; }
    [[nodiscard]] Caller* GetCaller() const noexcept { return caller; }
    [[nodiscard]] RelicWeaponState* GetCurrentState() const noexcept { return currentState.get(); }
    [[nodiscard]] const RecentHitList<RE::ActorHandle>& GetLastHitActors() const noexcept { return lastHitActors; }
    [[nodiscard]] const RecentHitList<RE::ObjectRefHandle>& GetLastHitForms() const noexcept { return lastHitForms; }

    void GetPosition(RE::NiPoint3& a_point);
    ActorCapability* GetActiveController() const noexcept;

    void Update(float a_delta) override;
    void UpdateProjectile(RE::Projectile* a_projectile, float a_delta) override;
    bool OnHit(RE::ArrowProjectile* a_projectile, RE::hkpAllCdPointCollector* a_collector) override;
    void PostImpact(RE::ArrowProjectile* a_projectile, RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void OnTrailDelete() override;

    Caller* caller = nullptr;
    BaseActor* callerParent = nullptr;
    std::unique_ptr<RelicWeaponState> currentState;
    RecentHitList<RE::ActorHandle> lastHitActors;
    RecentHitList<RE::ObjectRefHandle> lastHitForms;
    RE::NiPointer<RE::NiNode> stuckedBone;
    RE::ActorHandle stuckedActor;
    bool resetStateOnUpdate = false;

private:
    SmartRelicWeapon& smartWeapon;
    std::uint64_t delayedCallTaskID = 0u;
    RE::BGSProjectile* ArrivingWeaponDummyProjectile = nullptr;
    RE::TESAmmo* ArrivingWeaponDummyAmmo = nullptr;
};
