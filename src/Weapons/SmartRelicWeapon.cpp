#include "SmartRelicWeapon.h"
#include "ThrownSmartWeapon.h"
#include "Weapons/States/ArrivingState.h"
#include "Weapons/States/HomingState.h"

SmartRelicWeapon::SmartRelicWeapon(RE::TESBoundObject* a_object)
    : ThrowableRelicWeapon(a_object)
{
    abilities.set(RelicAbility::kThrowable, RelicAbility::kCallable, RelicAbility::kSmart);
    Initialize();
}

bool SmartRelicWeapon::Initialize()
{
    if (!ThrowableWeaponDummyProjectile || !ThrowableWeaponDummyAmmo) return false;
    ThrowableWeaponDummyProjectile->data.flags.reset(pbFlag::kCanPickUp);
    if (!ArrivingWeaponDummyProjectile)
        ArrivingWeaponDummyProjectile = CreateBaseProjectile("ArrivingWeaponProjectile", "Arriving Weapon Projectile");
    if (ArrivingWeaponDummyProjectile && !ArrivingWeaponDummyAmmo)
        ArrivingWeaponDummyAmmo = CreateBaseAmmo(ArrivingWeaponDummyProjectile, "ArrivingWeaponAmmo", "Arriving Weapon Ammo");
    if (ArrivingWeaponDummyProjectile)
        ArrivingWeaponDummyProjectile->data.flags.reset(RE::BGSProjectileData::BGSProjectileFlags::kCanPickUp);
    return ThrowableWeaponDummyAmmo && ArrivingWeaponDummyProjectile && ArrivingWeaponDummyAmmo;
}
std::unique_ptr<ThrownWeapon> SmartRelicWeapon::CreateThrownWeapon()
{
    return std::make_unique<ThrownSmartWeapon>(*this);
}

ThrownSmartWeapon* SmartRelicWeapon::GetLastThrownSmartWeapon() const noexcept
{
    return static_cast<ThrownSmartWeapon*>(GetLastThrownWeapon());
}
RelicWeaponState* SmartRelicWeapon::GetCurrentState() const noexcept
{
    const auto runtime = GetLastThrownSmartWeapon();
    return runtime ? runtime->GetCurrentState() : nullptr;
}
Caller* SmartRelicWeapon::GetCaller() const noexcept
{
    const auto runtime = GetLastThrownSmartWeapon();
    return runtime ? runtime->GetCaller() : nullptr;
}

void SmartRelicWeapon::SetState(RelicWeaponState::Type a_type)
{
    if (auto runtime = GetLastThrownSmartWeapon())
        runtime->SetState(a_type);
}
void SmartRelicWeapon::SetState(ThrownSmartWeapon& a_weapon, RelicWeaponState::Type a_type)
{
    switch (a_type) {
    case RelicWeaponState::Type::kArriving:
        a_weapon.SetState(std::make_unique<ArrivingState>(a_weapon));
        break;
    case RelicWeaponState::Type::kHoming:
        a_weapon.SetState(std::make_unique<HomingState>(a_weapon));
        break;
    case RelicWeaponState::Type::kNone:
    default:
        a_weapon.SetState(std::unique_ptr<RelicWeaponState>{});
        break;
    }
}

bool SmartRelicWeapon::Call(Caller* a_caller, const bool a_justDestroy, std::optional<float> a_delay)
{
    const auto runtime = GetLastThrownSmartWeapon();
    return runtime && runtime->Call(a_caller, a_justDestroy, a_delay);
}
void SmartRelicWeapon::Catch(const bool a_justDestroy)
{
    if (auto runtime = GetLastThrownSmartWeapon())
        runtime->Catch(a_justDestroy);
}

