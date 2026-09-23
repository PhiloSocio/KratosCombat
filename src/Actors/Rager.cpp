#include "Rager.h"

void Rager::StartRage(const bool a_justAnim)
{
    if (a_actor) {
        if ((*values.rage - *values.rageDamageAmount * 10.f) < 0.f) return;
        a_actor->SetGraphVariableInt("iRageType", Config::RageType);
        if (WeaponIdentify::EquippedObjR) _LastEquippedObjectR = WeaponIdentify::EquippedObjR;
        if (WeaponIdentify::EquippedObjL) _LastEquippedObjectL = WeaponIdentify::EquippedObjL;
        _lastTriggeredRage = a_rage;
        switch (a_rage)
        {
        case Kratos::Rage::kFury:
            if (a_justAnim) {
                a_actor->SetGraphVariableInt("iKratosActionType", (uint8_t)Kratos::Action::kRage);
                a_actor->NotifyAnimationGraph("DoKratosAction");
                return;
            }
            ObjectUtil::Actor::CastSpell(SpellSpartanRage, a_actor, a_actor, a_actor, 3.f);
            if (VFXeffect.fury) a_actor->ApplyArtObject(VFXeffect.fury, 1.f, nullptr, false, false, WeaponIdentify::GetRhandBone(a_actor));
            if (WeaponIdentify::EquippedObjR)
                ObjectUtil::Actor::UnEquipItem(a_actor, false, false, false, false, true, true);
            if (WeaponIdentify::EquippedObjL && WeaponIdentify::EquippedObjL != WeaponIdentify::GuardianShield)
                ObjectUtil::Actor::UnEquipItem(a_actor, true, false, false, false, true, true);
            if (WeaponIdentify::GuardianShield && !WeaponIdentify::isGuardianShield)
                ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::GuardianShield, true);

            ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
            break;
        case Kratos::Rage::kValor:
            if (a_justAnim) {
                _gettingHittedInValor = false;
                a_actor->SetGraphVariableInt("iKratosActionType", (uint8_t)Kratos::Action::kRage);
                a_actor->NotifyAnimationGraph("DoKratosAction");
                RestoreRage(a_actor, -(*values.rageDamageAmount * 5.f), true);
            }
            if (VFXeffect.valor) a_actor->ApplyArtObject(VFXeffect.valor, 1.f, nullptr, false, false, WeaponIdentify::GetRhandBone(a_actor));
            break;
        case Kratos::Rage::kWrath:
            if (VFXeffect.wrath) a_actor->ApplyArtObject(VFXeffect.wrath, 1.f, nullptr, false, false, WeaponIdentify::GetRhandBone(a_actor));
            return;
        case Kratos::Rage::kLegacy:
            if (WeaponIdentify::BladeOfOlympus) {
            //    if (auto mCaster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant); mCaster && SpellSpartanRage) {
            //        mCaster->CastSpellImmediate(SpellSpartanRage, false, a_actor, 1.f, false, 2.f, a_actor);
            //    }
                if (VFXeffect.legacy) a_actor->ApplyArtObject(VFXeffect.legacy, 1.f, nullptr, false, false, WeaponIdentify::GetRhandBone(a_actor));
                ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::BladeOfOlympus);
                ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
            }
            break;
        default:
            break;
        }
        ObjectUtil::Actor::SendAnimationEvent(a_actor, "weaponSwing");
        a_actor->SetGraphVariableBool("IsInRage", true);
    }
}
void Rager::EndRage(const bool a_fromAnnotation, const bool a_playAnim, const bool a_justAnim, RE::Actor* a_actor)
{
    if (a_actor) {
        _isWantFinishRage = true;
        switch (a_rage)
        {
        case Kratos::Rage::kFury:
            if (a_playAnim) {
                a_actor->NotifyAnimationGraph("DoKratosAction");
                _isWantFinishRage = false;
                if (a_justAnim) return;
            }
            if (auto mTarget = a_actor->GetMagicTarget(); mTarget) {
                auto aHandle = a_actor->GetHandle();
                mTarget->DispelEffect(SpellSpartanRage, aHandle);
                ObjectUtil::Actor::ResetEquipAnimationAfter(0, a_actor);
                RestoreRage(a_actor, -(*values.rageDamageAmount * 3.f), true);
                if (_LastEquippedObjectR)
                    ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectR, true);
                if (_LastEquippedObjectL)
                    ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectL, true);
                ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
            }
            break;
        case Kratos::Rage::kValor:
            if (a_fromAnnotation) {
                ObjectUtil::Actor::SendAnimationEvent(a_actor, "weaponSwing");
                a_actor->AsActorValueOwner()->RestoreActorValue(RE::ActorValue::kHealth, (*values.rageBuffAmount * 10.f));
                if (_gettingHittedInValor) ObjectUtil::Actor::CastSpell(SpellStrenghtBuff, a_actor, a_actor, a_actor);
                _gettingHittedInValor = false;
            }
            break;
        case Kratos::Rage::kWrath:
            break;
        case Kratos::Rage::kLegacy:
            if (a_playAnim) {
                a_actor->NotifyAnimationGraph("DoKratosAction");
                _isWantFinishRage = false;
                if (a_justAnim) return;
            }
            ObjectUtil::Actor::ResetEquipAnimationAfter(0, a_actor);
            if (_LastEquippedObjectR)
                ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectR, true);
            else if (WeaponIdentify::EquippedObjR) 
                ObjectUtil::Actor::UnEquipItem(a_actor, false, false, false, false, true, true);
            if (_LastEquippedObjectL)
                ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectL, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
            break;

        default:
            break;
        }
        a_actor->SetGraphVariableBool("IsInRage", false);
        _isWantFinishRage = false;
        _LastEquippedObjectR = nullptr;
        _LastEquippedObjectL = nullptr;
    }
}
