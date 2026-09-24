#include "Rager.h"
#include "Assets.h"
#include "util.h"

float Rager::CalcRageDamageOrBuffAmount(const float a_amount, const float a_mult)
{
    if (a_amount == 0.f) return 0.f;
    else if (a_amount > 0.f) {
        if (IsInRage()) return -(rageDamageAmount * a_mult * (5.f + a_amount / 300.f));
        else return (rageBuffAmount * a_mult * (1.f + sqrtf(a_amount) / 100.f));
    } return (rageDamageAmount * a_mult * (a_amount - 1.f) / 5.f);
}

void Rager::RestoreRage(const float a_value, const bool a_justRestore)
{
    rage += a_value;
    if (rage < 0.f) {rage = 0.f; if (!a_justRestore) EndRage(false, true, true);}
    else if (rage > rageLimit) rage = rageLimit;
}

void Rager::StartRage(const bool a_justAnim)
{
    if (actor) {
        if ((rage - rageDamageAmount * 10.f) < 0.f) return;
        actor->SetGraphVariableInt("iRageType", (int32_t)rageType);
        _LastEquippedObjectR = GetEquippedObjectR();
        _LastEquippedObjectL = GetEquippedObjectL();
        lastTriggeredRage = rageType;
        auto assets = Assets::GetSingleton();
        switch (rageType)
        {
        case RageType::kFury:
            if (a_justAnim) {
                actor->SetGraphVariableInt("iKratosActionType", (uint32_t)ActionType::kRage);
                actor->NotifyAnimationGraph("DoKratosAction");
                return;
            }
            ObjectUtil::Actor::CastSpell(SpellSpartanRage, actor, actor, actor, 3.f);
            if (assets->VFXeffects.fury) actor->ApplyArtObject(assets->VFXeffects.fury, 1.f, nullptr, false, false, GetRHandBone());
            if (GetEquippedObjectR())
                ObjectUtil::Actor::UnEquipItem(actor, false, false, false, false, true, true);
            if (GetEquippedObjectL())
                ObjectUtil::Actor::UnEquipItem(actor, true, false, false, false, true, true);

            ObjectUtil::Actor::ResetEquipAnimationAfter(100, actor);
            break;
        case RageType::kValor:
            if (a_justAnim) {
                _gettingHittedInValor = false;
                actor->SetGraphVariableInt("iKratosActionType", (uint8_t)ActionType::kRage);
                actor->NotifyAnimationGraph("DoKratosAction");
                RestoreRage(-(rageDamageAmount * 5.f), true);
            }
            if (assets->VFXeffects.valor) actor->ApplyArtObject(assets->VFXeffects.valor, 1.f, nullptr, false, false, GetRHandBone());
            break;
        case RageType::kWrath:
            if (assets->VFXeffects.wrath) actor->ApplyArtObject(assets->VFXeffects.wrath, 1.f, nullptr, false, false, GetRHandBone());
            return;
        case RageType::kLegacy:
        //    if (BladeOfOlympus) {
        //    //    if (auto mCaster = actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant); mCaster && SpellSpartanRage) {
        //    //        mCaster->CastSpellImmediate(SpellSpartanRage, false, actor, 1.f, false, 2.f, actor);
        //    //    }
        //        if (assets->VFXeffects.legacy) actor->ApplyArtObject(assets->VFXeffects.legacy, 1.f, nullptr, false, false, GetRhandBone(actor));
        //        ObjectUtil::Actor::EquipItem(actor, BladeOfOlympus);
        //        ObjectUtil::Actor::ResetEquipAnimationAfter(100, actor);
        //    }
            break;
        default:
            break;
        }
        ObjectUtil::Actor::SendAnimationEvent(actor, "weaponSwing");
        actor->SetGraphVariableBool("IsInRage", true);
        _isInRage = true;
    }
}
void Rager::EndRage(const bool a_fromAnnotation, const bool a_playAnim, const bool a_justAnim)
{
    if (actor) {
        _isWantFinishRage = true;
        switch (rageType)
        {
        case RageType::kFury:
            if (a_playAnim) {
                actor->NotifyAnimationGraph("DoKratosAction");
                _isWantFinishRage = false;
                if (a_justAnim) return;
            }
            if (auto mTarget = actor->GetMagicTarget(); mTarget) {
                auto aHandle = actor->GetHandle();
                mTarget->DispelEffect(SpellSpartanRage, aHandle);
                ObjectUtil::Actor::ResetEquipAnimationAfter(0, actor);
                RestoreRage(-(rageDamageAmount * 3.f), true);
                if (_LastEquippedObjectR)
                    ObjectUtil::Actor::EquipItem(actor, _LastEquippedObjectR, true);
                if (_LastEquippedObjectL)
                    ObjectUtil::Actor::EquipItem(actor, _LastEquippedObjectL, true);
                ObjectUtil::Actor::ResetEquipAnimationAfter(100, actor);
            }
            break;
        case RageType::kValor:
            if (a_fromAnnotation) {
                ObjectUtil::Actor::SendAnimationEvent(actor, "weaponSwing");
                actor->AsActorValueOwner()->RestoreActorValue(RE::ActorValue::kHealth, (rageBuffAmount * 10.f));
                if (_gettingHittedInValor) ObjectUtil::Actor::CastSpell(SpellStrenghtBuff, actor, actor, actor);
                _gettingHittedInValor = false;
            }
            break;
        case RageType::kWrath:
            break;
        case RageType::kLegacy:
            if (a_playAnim) {
                actor->NotifyAnimationGraph("DoKratosAction");
                _isWantFinishRage = false;
                if (a_justAnim) return;
            }
            if (GetEquippedObjectR()) {
                ObjectUtil::Actor::UnEquipItem(actor, false, false, false, false, true, true);
                ObjectUtil::Actor::ResetEquipAnimationAfter(100, actor);
            }
            break;

        default:
            break;
        }
        actor->SetGraphVariableBool("IsInRage", false);
        _isInRage = false;
        _isWantFinishRage = false;
        _LastEquippedObjectR = nullptr;
        _LastEquippedObjectL = nullptr;
    }
}

void Rager::UpdateRager(const float a_delta)
{
    if (IsInRage()) RestoreRage(-(rageDamageAmount * a_delta), true);
}
