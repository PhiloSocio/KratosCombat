#pragma once

enum class ActorType : std::uint32_t {
    kNone = 0,
    kThrower = 1 << 0,
    kCaller = 1 << 1,
    kCharger = 1 << 2,
    kRager = 1 << 3
};

enum class ActionType : std::uint8_t {
    kRage,
    kWeaponCharge,
    kSpecialIdle,
    kWeaponCall
};

enum class RageType : std::uint8_t {
    kFury,
    kValor,
    kWrath,
    kLegacy
};

enum class RelicType : std::uint8_t {
    kNone,
    kLeviathanAxe,
    kBladesOfChaos,
    kDraupnir,
    kMjolnir,
    kTrident,
    kCount
};

enum class ShieldType : std::uint8_t {
    kNone,
    kGuardianShield,
    kDauntlessShield,
    kStoneWallShield,
    kShatterStarShield,
    kOnslaughtShield
};

enum class RotationType : std::uint8_t {
    kNone,
    kSpinLateral,
    kSpinVertical
};

enum class ImpactType : std::uint8_t {
    kSharp,
    kBlunt
};

enum class ThrowType : std::uint8_t {
    kAny,
    kNormalThrowing,
    kPowerThrowing,
    kChargingThrow,
    kPowerChargingThrow
};
