#include "RelicWeapon.h"

void RelicWeapon::SetWielder(BaseActor* a_actor) {
    if (a_actor && a_actor->IsValid()) {
        runtimeData.wielder = a_actor;
        runtimeData.lastWielder = a_actor;
        runtimeData.isEquipped = true;
    } else {
        runtimeData.wielder = nullptr;
        runtimeData.isEquipped = false;
    }
}