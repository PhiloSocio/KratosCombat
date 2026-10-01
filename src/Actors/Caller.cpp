#include "Caller.h"
#include "Weapons/SmartRelicWeapon.h"
#include "RelicManager.h"

void Caller::CallWeapon()
{
    if (auto weaponToCall = dynamic_cast<SmartRelicWeapon*>(_weaponToCall)) {
        weaponToCall->Call(this);
    } else {
        spdlog::info("no callable weapon found!");
    }
}
RelicWeapon* Caller::GetCallableRelic()
{
    _weaponToCall = nullptr;
    if (!GetRightHandRelic()) {
        if (auto lastRelic = GetLastRightHandRelic();
            lastRelic && !lastRelic->IsEquipped() && 
            lastRelic->GetOwner() && 
            lastRelic->GetOwner() == this && 
            lastRelic->HasAbility(RelicAbility::kCallable))
        {
            _weaponToCall = lastRelic;
        } else if (auto& knownRelics = GetKnownRelics(); !knownRelics.empty()) {
            for (const auto& relicID : knownRelics) {
                auto relic = RelicManager::GetSingleton()->GetActiveRelic(relicID);
                if (!relic || !relic->HasAbility(RelicAbility::kCallable)) continue;
                if (auto owner = relic->GetOwner()) {
                    if (owner == this) {
                        if (!relic->IsEquipped()) {
                            _weaponToCall = relic;
                            break;
                        } else if (auto wielder = relic->GetWielder()) {
                            if (this->alterationLevel > wielder->meleeSkill * 2.f) {
                                _weaponToCall = relic;
                                break;
                            }
                        } else {
                            spdlog::error("WEIRD! Relic is not owned by this actor, but it's equipped. This should never happen.");
                        }
                    } else if (auto wielder = relic->GetWielder()) {
                        if (this->alterationLevel > wielder->meleeSkill * 3.f) {
                            _weaponToCall = relic;
                            break;
                        }
                    } else {
                        _weaponToCall = relic;
                    }
                } else {
                    _weaponToCall = relic;
                    break;
                }
            }
        } else {
            spdlog::info("you don't have any relic weapon to call.");
        }
    } else {
        spdlog::info("you already have a relic weapon equipped.");
    }
    return _weaponToCall;
}
