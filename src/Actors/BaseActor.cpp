#include "BaseActor.h"
#include "util.h"

RE::NiAVObject*& BaseActor::GetUpdateRHandBone()
{
    if      (RHandBone = ObjectUtil::Actor::GetBoneByName(actor, "NPC R Finger20 [RF20]"); RHandBone)  {}
    else if (RHandBone = ObjectUtil::Actor::GetBoneByName(actor, "NPC R MagicNode [RMag]"); RHandBone) {}
    else if (RHandBone = ObjectUtil::Actor::GetBoneByName(actor, "NPC R Hand [RHnd]"); RHandBone)      {}
    else    spdlog::error("can't found right hand bone!");
    return RHandBone;
}
RE::NiAVObject*& BaseActor::GetUpdateLHandBone()
{
    if      (LHandBone = ObjectUtil::Actor::GetBoneByName(actor, "NPC L Finger20 [LF20]"); LHandBone)  {}
    else if (LHandBone = ObjectUtil::Actor::GetBoneByName(actor, "NPC L MagicNode [LMag]"); LHandBone) {}
    else if (LHandBone = ObjectUtil::Actor::GetBoneByName(actor, "NPC L Hand [LHnd]"); LHandBone)      {}
    else    spdlog::error("can't found right hand bone!");
    return LHandBone;
}
RE::NiAVObject*& BaseActor::GetUpdateWeaponBone()
{
    WeaponBone = ObjectUtil::Actor::GetBoneByName(actor, "Weapon");
    return WeaponBone;
}
RE::NiAVObject*& BaseActor::GetUpdateShieldBone()
{
    ShieldBone = ObjectUtil::Actor::GetBoneByName(actor, "Shield");
    return ShieldBone;
}
RE::NiAVObject*& BaseActor::GetUpdateAnimObjectRBone()
{
    AnimObjectRBone = ObjectUtil::Actor::GetBoneByName(actor, "AnimObjectR");
    return AnimObjectRBone;
}

void BaseActor::OnEquip(RelicWeapon* a_relic)
{
    equippedObjectR = actor->GetEquippedObject(false) ? actor->GetEquippedObject(false)->As<RE::TESBoundObject>() : nullptr;
    equippedObjectL = actor->GetEquippedObject(true) ? actor->GetEquippedObject(true)->As<RE::TESBoundObject>() : nullptr;
    rightHandRelic = a_relic;
    lastRightHandRelic = a_relic;
    knownRelics.emplace_back(a_relic);
}
