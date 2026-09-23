#pragma once
#include "util.h"
#include "settings.h"

class BladeOfChaos
{
public:
    static  BladeOfChaos* GetSingleton() {static BladeOfChaos singleton; return &singleton;}
    bool Initialize();

    enum class WeaponState : std::uint_fast8_t {
        kNone = 0,
        kChainClose = 1,
        kChainOpen = 2
    };

    struct Data {
        RE::TESObjectWEAP* weap     = nullptr;
        RE::TESObjectWEAP* weapL    = nullptr;
        RE::EnchantmentItem* ench   = nullptr;
        RE::AlchemyItem* poison     = nullptr;
        RE::NiAVObject* weaponModel = nullptr;
        RE::NiAVObject* weaponModelL= nullptr;
        RE::NiNode* stuckedBone     = nullptr;
        RE::Actor* stuckedActor     = nullptr;
        RE::NiNode* stuckedBoneL    = nullptr;
        RE::Actor* stuckedActorL    = nullptr;
        float* enchMag              = nullptr;
        float defaultEnchMag;
        float damage            = 0.f;
    };
    Data data;

    void    Update(const float a_delta) {_lastChargeTime = AsyncUtil::GameTime::GetEngineTime();}
    void    SetWeaponState(const WeaponState a_weaponState);
    WeaponState GetWeaponState() const;
    bool    IsScorching() const {return _isScorching;}
    void    SetIsScorching(const bool a_isScorching = true) {_isScorching = a_isScorching; RE::PlayerCharacter::GetSingleton()->SetGraphVariableBool("bIsScorching", a_isScorching);}
    float   GetScorchingSpeed();
    void    SetScorchingSpeed(const float a_speed, const bool a_forced = false);
    void    BuffScorchingSpeed(const float a_buff = 0.05f, const bool a_forced = false);
    void    DeBuffScorchingSpeed();
    bool    IsQueueEnd();
    void    HideChains(const bool a_hide = true);
private:
    WeaponState weaponState;
    bool    _isChainHidden = false;
    bool    _isScorching = false;
    float   _fScorchingSpeed = 0.5f;
    float   _lastChargeTime;
};

using wStateB = BladeOfChaos::WeaponState;
