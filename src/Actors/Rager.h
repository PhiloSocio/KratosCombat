#pragma once

#include "BaseActor.h"

class Rager : virtual public BaseActor
{
public:
    explicit Rager(RE::ActorHandle a_actorHandle) :
        BaseActor(a_actorHandle)
    {titles.set(ActorType::kRager);}
    virtual ~Rager() = default;

    void UpdateRager(float a_delta);

    void StartRage(const bool a_justAnim = false);
    void EndRage(const bool a_fromAnnotation = false, const bool a_playAnim = true, const bool a_justAnim = false);

    float CalcRageDamageOrBuffAmount(const float a_amount, const float a_mult = 1.f);
    void RestoreRage(const float a_value, const bool a_justRestore);

    [[nodiscard]] bool IsInRage() const { return _isInRage; }
    [[nodiscard]] bool IsCanRage() const { return (rage > rageDamageAmount * 6.f) && !_isInRage; }
    [[nodiscard]] bool IsWantFinishRage() const {return _isWantFinishRage;}

    [[nodiscard]] RageType GetRageType() const { return rageType; }
    [[nodiscard]] float GetRageAmount() const { return rage; }
    [[nodiscard]] float GetRageLimit() const { return rageLimit; }
    [[nodiscard]] float GetBuffAmount() const { return rageBuffAmount; }
    [[nodiscard]] float GetRageDamageAmount() const { return rageDamageAmount; }

    void SetRageAmount(float a_rageAmount) noexcept { rage = a_rageAmount; }
    void SetRageLimit(float a_rageLimit) noexcept { rageLimit = a_rageLimit; }
    void SetRageBuffAmount(float a_rageBuffAmount) noexcept { rageBuffAmount = a_rageBuffAmount; }
    void SetRageDamageAmount(float a_rageDamageAmount) noexcept { rageDamageAmount = a_rageDamageAmount; }
    void SetRageType(RageType a_rageType) noexcept { rageType = a_rageType; }

protected:
    RE::SpellItem*      SpellSpartanRage    = nullptr;
    RE::SpellItem*      SpellStrenghtBuff   = nullptr;
    RE::EffectSetting*  EffectSpartanRage   = nullptr;
    RE::EffectSetting*  EffectStrenghtBuff  = nullptr;

    float rage             = 0.f;
    float rageLimit        = 0.f;
    float rageBuffAmount   = 0.f;
    float rageDamageAmount = 0.f;

    RageType rageType = RageType::kFury;
    RageType lastTriggeredRage = RageType::kFury;

private:
    RE::TESBoundObject* _LastEquippedObjectR = nullptr;
    RE::TESBoundObject* _LastEquippedObjectL = nullptr;
    bool _isInRage             = false;
    bool _isWantFinishRage     = false;
    bool _gettingHittedInValor = false;
};
