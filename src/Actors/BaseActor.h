#pragma once

#include "Types.h"

class RelicWeapon;

class BaseActor
{
public:
    virtual ~BaseActor() = default;

    void OnEquip(RelicWeapon* a_relic);

    virtual void Update(float a_delta) = 0;
    virtual void DoAction(const ActionType a_action) = 0;

    void SetRightHandRelic(RelicWeapon* a_relic) { rightHandRelic = a_relic; };
    void SetLeftHandRelic(RelicWeapon* a_relic) { leftHandRelic = a_relic; };

    void SetLastRightHandRelic(RelicWeapon* a_relic) { lastRightHandRelic = a_relic; };
    void SetLastLeftHandRelic(RelicWeapon* a_relic) { lastLeftHandRelic = a_relic; };

    void SetDamageMult(const float a_damageMult) { damageMult = a_damageMult; };
    void SetSkipEquipAnim(const bool a_skip) { skipEquipAnim = a_skip; };
    void SetUnequipWhenAnimEnds(const bool a_unequip) { unequipWhenAnimEnds = a_unequip; };
    void SetBarehanded(const bool a_isBarehanded) { isBarehanded = a_isBarehanded; };
    void SetTitles(REX::EnumSet<ActorType, std::uint32_t> a_titles) { titles = a_titles; };
    void SetTitles(std::uint32_t a_titles) { titles = static_cast<ActorType>(a_titles); };

    [[nodiscard]] RE::Actor* GetActor() const { return actor; };
    [[nodiscard]] const RE::ActorHandle& GetActorHandle() const { return actorHandle; };

    [[nodiscard]] bool IsValid() { actor = actorHandle ? actorHandle.get().get() : nullptr; return actor != nullptr; };

    [[nodiscard]] bool HasTitle(const ActorType a_type) const { return titles.all(a_type); }
    [[nodiscard]] REX::EnumSet<ActorType, std::uint32_t> GetTitles() const { return titles; }

//  not safe to use, the bones should be updated every frame, otherwise they could ve dangling pointers
//    [[nodiscard]] RE::NiAVObject* GetRHandBone() const { return RHandBone; };
//    [[nodiscard]] RE::NiAVObject* GetLHandBone() const { return LHandBone; };
//    [[nodiscard]] RE::NiAVObject* GetWeaponBone() const { return WeaponBone; };
//    [[nodiscard]] RE::NiAVObject* GetShieldBone() const { return ShieldBone; };
//    [[nodiscard]] RE::NiAVObject* GetAnimObjectRBone() const { return AnimObjectRBone; };

    [[nodiscard]] RE::NiAVObject*& GetUpdateRHandBone();
    [[nodiscard]] RE::NiAVObject*& GetUpdateLHandBone();
    [[nodiscard]] RE::NiAVObject*& GetUpdateWeaponBone();
    [[nodiscard]] RE::NiAVObject*& GetUpdateShieldBone();
    [[nodiscard]] RE::NiAVObject*& GetUpdateAnimObjectRBone();
    [[nodiscard]] RE::NiAVObject*& GetRHandBone() { return GetUpdateRHandBone(); };
    [[nodiscard]] RE::NiAVObject*& GetLHandBone() { return GetUpdateLHandBone(); };
    [[nodiscard]] RE::NiAVObject*& GetWeaponBone() { return GetUpdateWeaponBone(); };
    [[nodiscard]] RE::NiAVObject*& GetShieldBone() { return GetUpdateShieldBone(); };
    [[nodiscard]] RE::NiAVObject*& GetAnimObjectRBone() { return GetUpdateAnimObjectRBone(); };

    [[nodiscard]] RelicWeapon* GetRightHandRelic() const { return rightHandRelic; };
    [[nodiscard]] RelicWeapon* GetLeftHandRelic() const { return leftHandRelic; };

    [[nodiscard]] RelicWeapon* GetLastRightHandRelic() const { return lastRightHandRelic; };
    [[nodiscard]] RelicWeapon* GetLastLeftHandRelic() const { return lastLeftHandRelic; };

    [[nodiscard]] std::vector<RelicWeapon*>& GetKnownRelics() { return knownRelics; };

    [[nodiscard]] bool IsBarehanded() const { return isBarehanded; };
    [[nodiscard]] float GetDamageMult() const { return damageMult; };
    [[nodiscard]] bool GetSkipEquipAnim() const { return skipEquipAnim; };
    [[nodiscard]] bool GetUnequipWhenAnimEnds() const { return unequipWhenAnimEnds; };
protected:
    RE::ActorHandle actorHandle;
    RE::Actor* actor = nullptr;

    RE::NiAVObject* RHandBone = nullptr;
    RE::NiAVObject* LHandBone = nullptr;
    RE::NiAVObject* WeaponBone = nullptr;
    RE::NiAVObject* ShieldBone = nullptr;
    RE::NiAVObject* AnimObjectRBone = nullptr;

    RelicWeapon* rightHandRelic = nullptr;
    RelicWeapon* leftHandRelic = nullptr;

    RelicWeapon* lastRightHandRelic = nullptr;
    RelicWeapon* lastLeftHandRelic = nullptr;

    std::vector<RelicWeapon*> knownRelics;

    REX::EnumSet<ActorType, std::uint32_t> titles;

    bool skipEquipAnim = false;
    bool unequipWhenAnimEnds = false;
    bool isBarehanded = false;

    explicit BaseActor(RE::ActorHandle a_actorHandle) :
        actorHandle(a_actorHandle)
    {}

public:
    float level = 0.f;
    float meleeSkill = 0.f;
    float damageMult = 1.f;
};
