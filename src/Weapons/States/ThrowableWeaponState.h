#pragma once
#include "RelicWeaponState.h"
#include "Weapons/SmartRelicWeapon.h"

class ThrowableWeaponState : public RelicWeaponState
{
public:
    explicit ThrowableWeaponState(SmartRelicWeapon& a_weapon) :
        weapon(a_weapon)
    {}
    ~ThrowableWeaponState() override = default;

protected:
    SmartRelicWeapon& weapon;

};
