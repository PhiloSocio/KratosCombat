#pragma once
#include "settings.h"
#include "hook.h"
class WeaponIdentify
{
public:
    static inline bool isLeviathanAxe;
    static inline bool isBladeOfChaos;
    static inline bool isDraupnirSpear;
    
    static inline RE::NiAVObject* RHandBone         = nullptr;
    static inline RE::TESObjectWEAP* RHandWeapon    = nullptr;
    static inline RE::TESObjectWEAP* LeviathanAxe   = nullptr;
    static inline RE::TESObjectWEAP* BladeOfChaos   = nullptr;
    static inline RE::TESObjectWEAP* DraupnirSpear  = nullptr;
    
    static void WeaponCheck();

private:

    static void WeaponIdentifier(RE::TESObjectWEAP* a_RHandWeapon);
};

class Leviathan
{
public:
    enum class ThrowState : std::uint8_t
    {
        kNone = 0,
        kThrowable = 1,
        kThrown = 2,
        kCanArrive = 3,
        kArriving = 4,
        kArrived = 5,
    };
    static inline ThrowState throwState;

    static inline RE::NiPoint3 leviPosition;// = {0.f, 0.f, 0.f};
    static inline float throwedTime;
    static inline float arrivalSpeed;

    static inline RE::Projectile::LaunchData LeviThrowData;// = nullptr;

    static inline RE::Projectile* LeviathanAxeProjectileL   = nullptr;
    static inline RE::Projectile* LeviathanAxeProjectileH   = nullptr;

    static inline RE::TESObjectREFR* ThrowedLevi    = nullptr;
    static inline RE::NiAVObject* ThrowedLevi1      = nullptr;
    static inline RE::NiAVObject* ThrowedLevi2      = nullptr;
    static inline RE::NiNode* ThrowedLeviNode       = nullptr;

    static inline RE::BGSProjectile* LeviProjBaseL  = nullptr;  //  ligth attack throw
    static inline RE::BGSProjectile* LeviProjBaseH  = nullptr;  //  heavy attack throw
    static inline RE::BGSProjectile* LeviProjBaseA  = nullptr;  //  arriving leviathan
    static inline RE::SpellItem* SpellLeviProjL     = nullptr;  //  light throw
    static inline RE::SpellItem* SpellLeviProjH     = nullptr;  //  heavy throw
    static inline RE::SpellItem* SpellLeviProjA     = nullptr;  //  call levi

    static inline RE::SpellItem* SpellCatchLevi     = nullptr;
    static inline RE::EffectSetting* EffCatchLevi   = nullptr;

    static inline RE::EnchantmentItem* EnchCharge   = nullptr;

    static inline bool isAxeCalled;
    static inline bool isAxeThrowed;
    static inline bool isAxeStucked;

    static ThrowState GetThrowState();
    static void SetThrowState(const ThrowState a_throwState);
    static void Throw(bool isVertical);
    static void Arrive();
    static void Charge(const int a_DurationSec, const float a_Magnitude);
};

class Draupnir
{
public:
    static inline RE::SpellItem* SpellDraupnirProjL             = nullptr;
    static inline RE::Projectile* DraupnirSpearProjectiles[9];                          //they will be explode after Draupnir's Call move
    static inline RE::Projectile* DraupnirSpearProjectileL      = nullptr;
    static inline RE::BGSProjectile* DraupnirSpearProjBaseL     = nullptr;

    static inline RE::BGSExplosion* StuckedDraupnir             = nullptr;
    static inline RE::BGSExplosion* DraupnirExplosion           = nullptr;

    static void Throw();
    static void Call(float a_magnitude);
};

using EventChecker = RE::BSEventNotifyControl;
class AnimationEventTracker : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
public:
    static AnimationEventTracker* GetSingleton() {static AnimationEventTracker singleton; return &singleton;};

	static void Register();

    virtual EventChecker ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) override;
};

class InputEventTracker : public RE::BSTEventSink<RE::InputEvent*>
{
public:
    static InputEventTracker* GetSingleton() {static InputEventTracker singleton; return &singleton;}

    static void Register();

    virtual EventChecker ProcessEvent(RE::InputEvent* const *a_event, RE::BSTEventSource<RE::InputEvent*> *a_eventSource) override;

    static inline uint32_t  AimButton      = 0u;
    static inline uint32_t  AxeCallButton  = 0u;
};

/**/
class SpellCastTracker : public RE::BSTEventSink<RE::TESSpellCastEvent>
{
public:
//    static SpellCastTracker* GetSingleton() {static SpellCastTracker singleton; return &singleton;};
//
//	static void Register();
//
//    virtual EventChecker ProcessEvent(const RE::TESSpellCastEvent* a_event, RE::BSTEventSource<RE::TESSpellCastEvent>* a_eventSource) override;

    static inline RE::SpellItem*        SpellAimButton      = nullptr;
    static inline RE::SpellItem*        SpellAxeCallButton  = nullptr;
    static inline RE::EffectSetting*    EffectAimButton     = nullptr;
    static inline RE::EffectSetting*    EffectAxeCallButton = nullptr;
};
