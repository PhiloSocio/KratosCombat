#include "RelicWeapon.h"

RelicWeapon::RelicWeapon(RE::TESBoundObject* a_object)
    : weap(a_object->As<RE::TESObjectWEAP>())
{
}

void RelicWeapon::OnEquip(BaseActor* a_actor) {
    if (a_actor && a_actor->IsValid()) {
        wielder = a_actor;
        lastWielder = a_actor;
        isEquipped = true;
    } else {
        wielder = nullptr;
        isEquipped = false;
    }
}
void RelicWeapon::SetWielder(BaseActor* a_actor) {
    if (a_actor && a_actor->IsValid()) {
        wielder = a_actor;
        lastWielder = a_actor;
        isEquipped = true;
    } else {
        wielder = nullptr;
        isEquipped = false;
    }
}