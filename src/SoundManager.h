#pragma once
#include "util.h"
#include "settings.h"
#include "Weapons/ThrowableRelicWeapon.h"

struct SoundDefinition
{
    RE::BGSSoundDescriptorForm* chargingLoop0   = nullptr;
    RE::BGSSoundDescriptorForm* throwingStart   = nullptr;
    RE::BGSSoundDescriptorForm* throwingLoop0   = nullptr;
    RE::BGSSoundDescriptorForm* throwingLoop1   = nullptr;
    RE::BGSSoundDescriptorForm* callStart       = nullptr;
    RE::BGSSoundDescriptorForm* arrivingStart   = nullptr;
    RE::BGSSoundDescriptorForm* arrivingLoop0   = nullptr;
    RE::BGSSoundDescriptorForm* arrivingLoop1   = nullptr;
    RE::BGSSoundDescriptorForm* arrivingLoop2   = nullptr;
    RE::BGSSoundDescriptorForm* arrivingNear    = nullptr;
    RE::BGSSoundDescriptorForm* catching        = nullptr;
};

class SoundManager
{
public:
    enum class State : std::uint8_t {
        kNone = 0,
        kTriggered = 1,
        kPlaying = 2,
        kFading = 3,
        kPaused = 4,
        kStopped = 5
    };
    enum class SoundName : std::uint16_t {
        kNone,
        kChargingLoop,
        kThrowingStart,
        kThrowingLoop,
        kCallStart,
        kArrivingStart,
        kArrivingLoop,
        kArrivingNear,
        kCatch
    };

    std::unordered_map<SoundName, State> soundState;

    AsyncUtil::GameTime chargingLoopStopUpdate;
    AsyncUtil::GameTime throwingLoopStopUpdate;
    AsyncUtil::GameTime arrivingLoopStopUpdate;

    ThrowableRelicWeapon::RuntimeData weaponData;
    SoundDefinition soundDefinition;

    RE::BSSoundHandle ChargingLoop0SH;
    RE::BSSoundHandle ThrowingStartSH;
    RE::BSSoundHandle ThrowingLoop0SH;
    RE::BSSoundHandle ThrowingLoop1SH;
    RE::BSSoundHandle CallStartSH;
    RE::BSSoundHandle ArrivingStartSH;
    RE::BSSoundHandle ArrivingLoop0SH;
    RE::BSSoundHandle ArrivingLoop1SH;
    RE::BSSoundHandle ArrivingLoop2SH;
    RE::BSSoundHandle ArrivingNearSH;
    RE::BSSoundHandle CatchSH;

    void StopAllSounds();
    void PauseAllSounds();
    void ContinueAllSounds();
    void FadeAllSounds(const uint16_t a_durationMS = 500u);
    void PauseAllLoopingSounds();
    void ContinueAllLoopingSounds();

    void PlayChargingLoopSounds(RE::NiAVObject* a_source);
    void PlayThrowingSounds(RE::NiAVObject* a_source);
    void PlayThrowingLoopSounds(RE::NiAVObject* a_source);
    void PlayCallingHandSounds(RE::NiAVObject* a_source);
    void PlayArrivingStartSounds(RE::NiAVObject* a_source);
    void PlayArrivingLoopSounds(RE::NiAVObject* a_source);
    void PlayArrivingNearSounds(RE::NiAVObject* a_source);
    void PlayCatchingSounds(RE::NiAVObject* a_source);

    void FadeChargingLoopSounds(const uint16_t a_durationMS = 1000u);
    void FadeThrowingLoopSounds(const uint16_t a_durationMS = 1000u);
    void FadeCallingHandSounds(const uint16_t a_durationMS = 1000u);
    void FadeArrivingStartSounds(const uint16_t a_durationMS = 1000u);
    void FadeArrivingLoopSounds(const uint16_t a_durationMS = 1000u);
    void FadeArrivingNearSounds(const uint16_t a_durationMS = 1000u);
    
    void StopChargingLoopSounds(const uint16_t a_delayMS = 0u);
    void StopThrowingLoopSounds(const uint16_t a_delayMS = 0u);
    void StopArrivingLoopSounds(const uint16_t a_delayMS = 0u);

    [[nodiscard]] RE::BSSoundHandle GetSoundHandle(const SoundName a_soundName) const {
        RE::BSSoundHandle ret;
        switch (a_soundName) {
        case SoundName::kCallStart:
            return CallStartSH;
        case SoundName::kArrivingStart:
            return ArrivingStartSH;
        case SoundName::kArrivingLoop:
            return ArrivingLoop0SH;
        case SoundName::kArrivingNear:
            return ArrivingNearSH;
        case SoundName::kCatch:
            return CatchSH;
        case SoundName::kThrowingStart:
            return ThrowingStartSH;
        case SoundName::kThrowingLoop:
            return ThrowingLoop0SH;
        } return ret;
    }
    void UpdateSoundState(const SoundName a_soundName) {
        const auto soundHandle = GetSoundHandle(a_soundName);
        if (soundHandle.IsPlaying()) soundState[a_soundName] = State::kPlaying;
        spdlog::debug("sound state {}", soundHandle.state.underlying());
    }
    [[nodiscard]] bool IsTriggered(const SoundName a_soundName) const {
        bool ret = false;
        if (auto it = soundState.find(a_soundName); it != soundState.end()) {
            const auto soundHandle = GetSoundHandle(a_soundName);
            const bool isTriggered = (*it).second == State::kTriggered;
            ret = isTriggered;
        } return ret;
    }
    [[nodiscard]] State GetState(const SoundName a_soundName) {
        return soundState[a_soundName];
    }
    [[nodiscard]] bool IsSoundValid(const SoundName a_soundName)  {
        switch (GetState(a_soundName)) {
        case State::kNone:
            return !IsTriggered(a_soundName);
        case State::kTriggered:
            return false;
        case State::kPlaying:
            return false;
        case State::kFading:
            return true;
        case State::kPaused:
            return true;
        case State::kStopped:
            return true;
        } return false;
    }

    SoundManager(SoundDefinition a_soundDefinition, ThrowableRelicWeapon::RuntimeData& a_weaponData) : soundDefinition(a_soundDefinition), weaponData(a_weaponData) {};
};