#pragma once

#include "ThrowableRelicWeapon.h"
#include "Weapons/States/RelicWeaponState.h"

class Caller;
class ThrownSmartWeapon;

class SmartRelicWeapon : public ThrowableRelicWeapon
{
public:
    explicit SmartRelicWeapon(RE::TESBoundObject* a_object);

    static constexpr RelicAbility kRequiredAbility = RelicAbility::kSmart;

    bool Initialize() override;

    [[nodiscard]] ThrownSmartWeapon* GetLastThrownSmartWeapon() const noexcept;
    [[nodiscard]] RelicWeaponState* GetCurrentState() const noexcept;
    [[nodiscard]] Caller* GetCaller() const noexcept;
    [[nodiscard]] RE::BGSProjectile* GetArrivingWeaponDummyProjectile() const noexcept { return ArrivingWeaponDummyProjectile; }
    [[nodiscard]] RE::TESAmmo* GetArrivingWeaponDummyAmmo() const noexcept { return ArrivingWeaponDummyAmmo; }

    void SetState(RelicWeaponState::Type a_type);
    virtual void SetState(ThrownSmartWeapon& a_weapon, RelicWeaponState::Type a_type);

    bool Call(Caller* a_caller, bool a_justDestroy = false, std::optional<float> a_delay = std::nullopt);
    void Catch(bool a_justDestroy = false);

protected:
    std::unique_ptr<ThrownWeapon> CreateThrownWeapon() override;

    RE::BGSProjectile* ArrivingWeaponDummyProjectile = nullptr;
    RE::TESAmmo* ArrivingWeaponDummyAmmo = nullptr;

};
