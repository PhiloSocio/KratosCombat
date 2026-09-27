#pragma once

#include "BaseActor.h"
#include "Weapons/RelicWeapon.h"
#include "settings.h"

class Charger : virtual public BaseActor
{
public:
    explicit Charger(RE::ActorHandle a_actorHandle) :
        BaseActor(a_actorHandle)
    {titles.set(ActorType::kCharger);}
    virtual ~Charger() = default;

    void StartWeaponCharging() {
        if (auto rHandRelic = GetRightHandRelic(); rHandRelic /*&& IsCanCharge(rHandRelic->type)*/) {
            rHandRelic->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 1u, -1u);
        } else {
            spdlog::info("the weapon is not chargeable");
        }
    }

    bool IsCanCharge(const RelicType a_relic) const
    {
        const auto index = static_cast<std::size_t>(a_relic);

        if (index >= canCharge.size()) {
            return false;
        }

        return canCharge[index];
    }

protected:
    std::array<bool, static_cast<std::size_t>(RelicType::kCount)> canCharge{};
};
