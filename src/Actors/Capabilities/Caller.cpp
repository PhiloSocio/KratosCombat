#include "Caller.h"
#include "Weapons/SmartRelicWeapon.h"
#include "RelicManager.h"

Caller::Caller()
{
}

void Caller::CallWeapon(SmartRelicWeapon* a_weaponToCall)
{
    if (parent && parent->IsValid()) {
        if (a_weaponToCall && a_weaponToCall->Call(this, false)) {
            RelicManager::GetSingleton()->OnRelicThrow(a_weaponToCall->projectile, a_weaponToCall);
        } else if (auto weaponToCall = dynamic_cast<SmartRelicWeapon*>(_weaponToCall)) {
            if (auto wielder = weaponToCall->GetWielder(); wielder && wielder->GetActor() != parent->GetActor()) {
            //    if (weaponToCall->Call(this, false, 1.618f)) {
            //        weaponToCall->currentReference = wielder->GetActor()->DropObject(weaponToCall->GetWeapon(), nullptr, 1, nullptr, nullptr).get().get();
                    spdlog::info("weapon forcefully called from {}", wielder->GetActor()->GetName());
            //    }
            } else if (weaponToCall->Call(this, false)) {
                RelicManager::GetSingleton()->OnRelicThrow(weaponToCall->projectile, weaponToCall);
            }
        } else {
            spdlog::info("no callable weapon found!");
        }
    }
}
RelicWeapon* Caller::GetCallableRelic()
{
    _weaponToCall = nullptr;
    if (!parent) return nullptr;
    if (!parent->GetRightHandRelic()) {
        if (auto lastRelic = parent->GetLastRightHandRelic();
            lastRelic && !lastRelic->IsEquipped() &&
            lastRelic->GetOwner() &&
            lastRelic->GetOwner() == parent &&
            lastRelic->HasAbility(RelicAbility::kCallable))
        {
            _weaponToCall = lastRelic;
        } else if (auto& knownRelics = parent->GetKnownRelics(); !knownRelics.empty()) {
            for (const auto& relicID : knownRelics) {
                auto relic = RelicManager::GetSingleton()->GetActiveRelic(relicID);
                if (!relic || !relic->HasAbility(RelicAbility::kCallable)) continue;
                if (auto owner = relic->GetOwner()) {
                    if (owner == parent) {
                        if (!relic->IsEquipped()) {
                            _weaponToCall = relic;
                            break;
                        } else if (auto wielder = relic->GetWielder()) {
                            if (alterationLevel > wielder->meleeSkill * 2.f) {
                                _weaponToCall = relic;
                                break;
                            }
                        } else {
                            spdlog::error("WEIRD! Relic is not owned by this actor, but it's equipped. This should never happen.");
                        }
                    } else if (auto wielder = relic->GetWielder()) {
                        if (alterationLevel > wielder->meleeSkill * 3.f) {
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

void Caller::OnAttach(BaseActor* a_actor)
{
    ActorCapability::OnAttach(a_actor);
    if (parent && parent->IsValid()) {
        if (auto callerAVO = parent->GetActor() ? parent->GetActor()->AsActorValueOwner() : nullptr) {
            alterationLevel = callerAVO->GetActorValue(RE::ActorValue::kAlteration);
        }
    }
}

void Caller::HandleAction(const ActionType a_action)
{
    switch (a_action)
    {
    case ActionType::kRage:
        break;
    case ActionType::kWeaponCharge:
        break;
    case ActionType::kSpecialIdle:
        break;
    case ActionType::kWeaponCall:
        if (!parent->IsInRage() && !parent->GetRightHandRelic()) {
            if (_weaponToCall = GetCallableRelic(); _weaponToCall) {
                parent->GetActor()->SetGraphVariableInt("iKratosActionType", (uint8_t)ActionType::kWeaponCharge);   //  intentional
                parent->GetActor()->NotifyAnimationGraph("DoKratosAction");
            }
        } else if (parent->IsInRage()) {
            //  todo
        }
        break;

    default:
        break;
    }
}
