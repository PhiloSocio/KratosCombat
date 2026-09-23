#pragma once
#include "RelicWeaponState.h"

template <class TWeapon>
class ThrowableWeaponState : public RelicWeaponState
{
protected:
    TWeapon& weapon;

    void Enter() override;
    Status Update(float a_delta) override;
    void Exit() override;

    virtual ~ThrowableWeaponState() = default;
public:
    explicit ThrowableWeaponState(TWeapon& a_weapon) :
        weapon(a_weapon)
    {}
};
