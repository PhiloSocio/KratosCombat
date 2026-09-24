#pragma once

#include "Weapons/Leviathan.h"
//#include "Weapons/BladesOfChaos.h"
//#include "Weapons/Draupnir.h"
//#include "Weapons/Mjolnir.h"

class RelicFactory {
public:
    static RelicFactory* GetSingleton() {
        static RelicFactory singleton;
        return &singleton;
    }

    static std::unique_ptr<RelicWeapon> Create(RelicType a_type, RE::TESBoundObject* a_object) {
        switch (a_type) {
        case RelicType::kLeviathanAxe:
            return std::make_unique<LeviathanAxe>(a_object);
/*
        case RelicType::kBladesOfChaos:
            return std::make_unique<BladesOfChaos>(a_object);

        case RelicType::kDraupnir:
            return std::make_unique<Draupnir>(a_object);

        case RelicType::kMjolnir:
            return std::make_unique<Mjolnir>(a_object);

        case RelicType::kBladeOfOlympus:
            return std::make_unique<BladeOfOlympus>(a_object);

        case RelicType::kTrident:
            return std::make_unique<Trident>(a_object);
*/

        default:
            return nullptr;
        }
    }
};
