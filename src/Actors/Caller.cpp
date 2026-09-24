#include "Caller.h"
#include "Weapons/RelicWeapon.h"

RelicWeapon* Caller::GetCallableRelic()
{
    RelicWeapon* weaponToCall = nullptr;
    if (!GetRightHandRelic()) {
        if (auto lastRelic = GetLastRightHandRelic(); lastRelic && !lastRelic->IsEquipped() && lastRelic->GetOwner() && lastRelic->GetOwner() == this) {
            weaponToCall = lastRelic;
        } else if (auto knownRelics = GetKnownRelics(); !knownRelics.empty()) {
            for (const auto& relic : knownRelics) {
                if (auto owner = relic->GetOwner()) {
                    if (owner == this) {
                        if (!relic->IsEquipped()) {
                            weaponToCall = relic;
                            break;
                        } else if (auto wielder = relic->GetWielder()) {
                            if (this->alterationLevel > wielder->meleeSkill * 2.f) {
                                weaponToCall = relic;
                                break;
                            }
                        } else {
                            spdlog::error("WEIRD! Relic is not owned by this actor, but it's equipped. This should never happen.");
                        }
                    } else if (auto wielder = relic->GetWielder()) {
                        if (this->alterationLevel > wielder->meleeSkill * 3.f) {
                            weaponToCall = relic;
                            break;
                        }
                    } else {
                        weaponToCall = relic;
                        break;
                    }
                }
            }
        }
    } return weaponToCall;
}
