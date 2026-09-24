#include "SoundManager.h"

void SoundManager::StopAllSounds()
{
    if (ChargingLoop0SH.IsPlaying()) ChargingLoop0SH.Stop();
    if (ThrowingStartSH.IsPlaying()) ThrowingStartSH.Stop();
    if (ThrowingLoop0SH.IsPlaying()) ThrowingLoop0SH.Stop();
    if (ThrowingLoop1SH.IsPlaying()) ThrowingLoop1SH.Stop();
    if (CallStartSH.IsPlaying()) CallStartSH.Stop();
    if (ArrivingStartSH.IsPlaying()) ArrivingStartSH.Stop();
    if (ArrivingLoop0SH.IsPlaying()) ArrivingLoop0SH.Stop();
    if (ArrivingLoop1SH.IsPlaying()) ArrivingLoop1SH.Stop();
    if (ArrivingLoop2SH.IsPlaying()) ArrivingLoop2SH.Stop();
    if (ArrivingNearSH.IsPlaying()) ArrivingNearSH.Stop();
    if (CatchSH.IsPlaying()) CatchSH.Stop();
}
void SoundManager::PauseAllSounds()
{
    if (ChargingLoop0SH.IsPlaying() && GetState(SoundName::kChargingLoop) == State::kTriggered) {StopChargingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kChargingLoop] = State::kPaused;}
//    if (CallStartSH.IsPlaying() && GetState(SoundName::kCallStart) == State::kTriggered) {CallStartSH.Pause(); soundState[SoundName::kCallStart] = State::kPaused;}
//    if (ArrivingStartSH.IsPlaying() && GetState(SoundName::kArrivingStart) == State::kTriggered) {ArrivingStartSH.Pause(); soundState[SoundName::kArrivingStart] = State::kPaused;}
    if (ArrivingLoop0SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop1SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop2SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingNearSH.IsPlaying() && GetState(SoundName::kArrivingNear) == State::kTriggered) {ArrivingNearSH.Pause(); soundState[SoundName::kArrivingNear] = State::kPaused;}
//    if (ThrowingStartSH.IsPlaying() && GetState(SoundName::kThrowingStart) == State::kTriggered) {ThrowingStartSH.Pause(); soundState[SoundName::kThrowingStart] = State::kPaused;}
    if (ThrowingLoop0SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kPaused;}
    else if (ThrowingLoop1SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kPaused;}
//    if (CatchSH.IsPlaying() && GetState(SoundName::kCatch) == State::kTriggered) {CatchSH.Pause(); soundState[SoundName::kCatch] = State::kPaused;}

//    if (CallStartSH.IsPlaying() && GetState(SoundName::kCallStart) == State::kTriggered) {CallStartSH.Pause(); soundState[SoundName::kCallStart] = State::kPaused;}
//    if (ArrivingStartSH.IsPlaying() && GetState(SoundName::kArrivingStart) == State::kTriggered) {ArrivingStartSH.Pause(); soundState[SoundName::kArrivingStart] = State::kPaused;}
//    if (ArrivingLoop0SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {ArrivingLoop0SH.Pause(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingLoop1SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {ArrivingLoop1SH.Pause(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingLoop2SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {ArrivingLoop2SH.Pause(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingNearSH.IsPlaying() && GetState(SoundName::kArrivingNear) == State::kTriggered) {ArrivingNearSH.Pause(); soundState[SoundName::kArrivingNear] = State::kPaused;}
//    if (ThrowingStartSH.IsPlaying() && GetState(SoundName::kThrowingStart) == State::kTriggered) {ThrowingStartSH.Pause(); soundState[SoundName::kThrowingStart] = State::kPaused;}
//    if (ThrowingLoop0SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {ThrowingLoop0SH.Pause(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
//    if (ThrowingLoop1SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {ThrowingLoop1SH.Pause(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
//    if (CatchSH.IsPlaying() && GetState(SoundName::kCatch) == State::kTriggered) {CatchSH.Pause(); soundState[SoundName::kCatch] = State::kPaused;}
}
void SoundManager::ContinueAllSounds()
{
    if (GetState(SoundName::kChargingLoop) == State::kPaused) {ChargingLoop0SH.Play(); soundState[SoundName::kChargingLoop] = State::kPlaying;}
    if (GetState(SoundName::kThrowingStart) == State::kPaused) {ThrowingStartSH.Play(); soundState[SoundName::kThrowingStart] = State::kPlaying;}
    if (GetState(SoundName::kThrowingLoop) == State::kPaused) {ThrowingLoop0SH.Play(); soundState[SoundName::kThrowingLoop] = State::kPlaying;}
    if (GetState(SoundName::kThrowingLoop) == State::kPaused) {ThrowingLoop1SH.Play(); soundState[SoundName::kThrowingLoop] = State::kPlaying;}
    if (GetState(SoundName::kCallStart) == State::kPaused) {CallStartSH.Play(); soundState[SoundName::kCallStart] = State::kPlaying;}
    if (GetState(SoundName::kArrivingStart) == State::kPaused) {ArrivingStartSH.Play(); soundState[SoundName::kArrivingStart] = State::kPlaying;}
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {ArrivingLoop0SH.Play(); soundState[SoundName::kArrivingLoop] = State::kPlaying;}
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {ArrivingLoop1SH.Play(); soundState[SoundName::kArrivingLoop] = State::kPlaying;}
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {ArrivingLoop2SH.Play(); soundState[SoundName::kArrivingLoop] = State::kPlaying;}
    if (GetState(SoundName::kArrivingNear) == State::kPaused) {ArrivingNearSH.Play(); soundState[SoundName::kArrivingNear] = State::kPlaying;}
    if (GetState(SoundName::kCatch) == State::kPaused) {CatchSH.Play(); soundState[SoundName::kCatch] = State::kPlaying;}
}
void SoundManager::PauseAllLoopingSounds()
{
    if (ChargingLoop0SH.IsPlaying() && GetState(SoundName::kChargingLoop) == State::kTriggered) {StopChargingLoopSounds(); soundState[SoundName::kChargingLoop] = State::kPaused;}
    if (ThrowingLoop0SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
    else if (ThrowingLoop1SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
    if (ArrivingLoop0SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop1SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop2SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
}
void SoundManager::ContinueAllLoopingSounds()
{
    if (GetState(SoundName::kChargingLoop) == State::kPaused && source) {PlayChargingLoopSounds(*source);}
    if (GetState(SoundName::kThrowingLoop) == State::kPaused && source) {PlayThrowingLoopSounds(*source);}// soundState[SoundName::kThrowingLoop] = State::kPlaying; spdlog::debug("ThrowingLoop sounds continuing...");}
    if (GetState(SoundName::kArrivingLoop) == State::kPaused && source) {PlayArrivingLoopSounds(*source);}// soundState[SoundName::kArrivingLoop] = State::kPlaying; spdlog::debug("ArrivingLoop sounds continuing...");}
}

void SoundManager::FadeAllSounds(const uint16_t a_durationMS)
{
    if (CallStartSH.IsPlaying()) CallStartSH.FadeOutAndRelease(a_durationMS);
    if (ArrivingStartSH.IsPlaying()) ArrivingStartSH.FadeOutAndRelease(a_durationMS);
    if (ArrivingLoop0SH.IsPlaying()) ArrivingLoop0SH.FadeOutAndRelease(a_durationMS);
    if (ArrivingLoop1SH.IsPlaying()) ArrivingLoop1SH.FadeOutAndRelease(a_durationMS);
    if (ArrivingLoop2SH.IsPlaying()) ArrivingLoop2SH.FadeOutAndRelease(a_durationMS);
    if (ArrivingNearSH.IsPlaying()) ArrivingNearSH.FadeOutAndRelease(a_durationMS);
    if (CatchSH.IsPlaying()) CatchSH.FadeOutAndRelease(a_durationMS);
}

void SoundManager::PlayChargingLoopSounds(RE::NiAVObject* a_source)
{
    source = &a_source;
    if (IsSoundValid(SoundName::kChargingLoop)) {
        if (auto soundEffect = soundDefinition.chargingLoop0; soundEffect) {
            ObjectUtil::Sound::PlaySound(soundEffect, ChargingLoop0SH, a_source, 0.5f);
            soundState[SoundName::kChargingLoop] = State::kTriggered;
        }
    } else {
        spdlog::debug("updating the following node of the charging loop sounds.");
        if (ThrowingLoop0SH.IsPlaying()) {
            ThrowingLoop0SH.SetObjectToFollow(a_source);
            ThrowingLoop0SH.Play();
        }
    }
}
void SoundManager::PlayThrowingSounds(RE::NiAVObject* a_source)
{
//    if (auto soundEffect = soundDefinition.throwingStart; soundEffect)
//        ThrowingStartSH = ObjectUtil::Sound::PlaySound(soundEffect, a_source, 5.f);
}
void SoundManager::PlayThrowingLoopSounds(RE::NiAVObject* a_source)
{
    source = &a_source;
    if (IsSoundValid(SoundName::kThrowingLoop)) {
        if (auto soundEffect = soundDefinition.throwingLoop0; soundEffect) {
            ObjectUtil::Sound::PlaySound(soundEffect, ThrowingLoop0SH, a_source, 0.5f);
            soundState[SoundName::kThrowingLoop] = State::kTriggered;
        }
    //    if (auto soundEffect = soundDefinition.throwingLoop1; soundEffect) {
    //        ObjectUtil::Sound::PlaySound(soundEffect, ThrowingLoop1SH, a_source, 0.5f);
    //        soundState[SoundName::kThrowingLoop] = State::kTriggered;
    //    }
    } else {
        spdlog::debug("updating the following node of the throwing loop sounds.");
        if (ThrowingLoop0SH.IsPlaying()) {
            ThrowingLoop0SH.SetObjectToFollow(a_source);
            ThrowingLoop0SH.Play();
        }// if (ThrowingLoop1SH.IsPlaying()) {
    //        ThrowingLoop1SH.SetObjectToFollow(a_source);
    //        ThrowingLoop1SH.Play();
    //    }
    }
}
void SoundManager::PlayCallingHandSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = soundDefinition.callStart; soundEffect)
        ObjectUtil::Sound::PlaySound(soundEffect, CallStartSH, a_source, 5.f);
}
void SoundManager::PlayArrivingStartSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = soundDefinition.arrivingStart; soundEffect && IsSoundValid(SoundName::kArrivingStart)) {
        ObjectUtil::Sound::PlaySound(soundEffect, ArrivingStartSH, a_source, 5.f);
        soundState[SoundName::kArrivingStart] = State::kTriggered;
    } else {
        if (ArrivingStartSH.IsPlaying())
            ArrivingStartSH.SetObjectToFollow(a_source);
    }
}
void SoundManager::PlayArrivingLoopSounds(RE::NiAVObject* a_source)
{
    source = &a_source;
    if (IsSoundValid(SoundName::kArrivingLoop)) {
        if (auto soundEffect = soundDefinition.arrivingLoop0; soundEffect) {
            ObjectUtil::Sound::PlaySound(soundEffect, ArrivingLoop0SH, a_source, 0.5f);
            soundState[SoundName::kArrivingLoop] = State::kTriggered;
        }
        if (auto soundEffect = soundDefinition.arrivingLoop1; soundEffect) {
            ObjectUtil::Sound::PlaySound(soundEffect, ArrivingLoop1SH, a_source, 0.5f);
            soundState[SoundName::kArrivingLoop] = State::kTriggered;
        }
    //    if (auto soundEffect = soundDefinition.arrivingLoop2; soundEffect) {
    //        ObjectUtil::Sound::PlaySound(soundEffect, ArrivingLoop2SH, a_source, 0.5f);
    //        soundState[SoundName::kArrivingLoop] = State::kTriggered;
    //    }
    } else {
        spdlog::debug("updating the following node of the arriving loop sounds.");
        if (ArrivingLoop0SH.IsPlaying()) {
            ArrivingLoop0SH.SetObjectToFollow(a_source);
            ArrivingLoop0SH.Play();
        } if (ArrivingLoop1SH.IsPlaying()) {
            ArrivingLoop1SH.SetObjectToFollow(a_source);
            ArrivingLoop1SH.Play();
        }// if (ArrivingLoop2SH.IsPlaying()) {
    //        ArrivingLoop2SH.SetObjectToFollow(a_source);
    //        ArrivingLoop2SH.Play();
    //    }
    }
}
void SoundManager::PlayArrivingNearSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = soundDefinition.arrivingNear; soundEffect && IsSoundValid(SoundName::kArrivingNear) ) {
        ObjectUtil::Sound::PlaySound(soundEffect, ArrivingNearSH, a_source, 5.f);
        soundState[SoundName::kArrivingNear] = State::kTriggered;
    } else {
        if (ArrivingNearSH.IsPlaying())
            ArrivingNearSH.SetObjectToFollow(a_source);
    }
}
void SoundManager::PlayCatchingSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = soundDefinition.catching; soundEffect)
        ObjectUtil::Sound::PlaySound(soundEffect, CatchSH, a_source, 5.f);
}

void SoundManager::FadeChargingLoopSounds(const uint16_t a_durationMS)
{
    soundState[SoundName::kChargingLoop] = State::kFading;
    if (ChargingLoop0SH.IsPlaying()) ChargingLoop0SH.FadeOutAndRelease(a_durationMS);
    else {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kChargingLoop] = State::kStopped;}

    StopChargingLoopSounds(a_durationMS);
}
void SoundManager::FadeThrowingLoopSounds(const uint16_t a_durationMS)
{
    soundState[SoundName::kThrowingLoop] = State::kFading;
    if (ThrowingLoop0SH.IsPlaying()) ThrowingLoop0SH.FadeOutAndRelease(a_durationMS);
    else {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kStopped;}
    if (ThrowingLoop1SH.IsPlaying()) ThrowingLoop1SH.FadeOutAndRelease(a_durationMS);
    else {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kStopped;}

    StopThrowingLoopSounds(a_durationMS);
}
void SoundManager::FadeCallingHandSounds(const uint16_t a_durationMS)
{
    if (CallStartSH.IsPlaying()) CallStartSH.FadeOutAndRelease(a_durationMS);
}
void SoundManager::FadeArrivingStartSounds(const uint16_t a_durationMS)
{
    
    soundState[SoundName::kArrivingStart] = State::kFading;
    if (ArrivingStartSH.IsPlaying()) ArrivingStartSH.FadeOutAndRelease(a_durationMS);
    else soundState[SoundName::kArrivingStart] = State::kStopped;
}
void SoundManager::FadeArrivingLoopSounds(const uint16_t a_durationMS)
{
    soundState[SoundName::kArrivingLoop] = State::kFading;
    if (ArrivingLoop0SH.IsPlaying()) ArrivingLoop0SH.FadeOutAndRelease(a_durationMS);
    else {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kStopped;}
    if (ArrivingLoop1SH.IsPlaying()) ArrivingLoop1SH.FadeOutAndRelease(a_durationMS);
    else {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kStopped;}
    if (ArrivingLoop2SH.IsPlaying()) ArrivingLoop2SH.FadeOutAndRelease(a_durationMS);
    else {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kStopped;}

    StopArrivingLoopSounds(a_durationMS);
}
void SoundManager::FadeArrivingNearSounds(const uint16_t a_durationMS)
{
    soundState[SoundName::kArrivingNear] = State::kFading;
    if (ArrivingNearSH.IsPlaying()) ArrivingNearSH.FadeOutAndRelease(a_durationMS);
    else soundState[SoundName::kArrivingNear] = State::kStopped;
}

void SoundManager::StopChargingLoopSounds(const uint16_t a_delayMS)
{
    if (a_delayMS == 0) {
        if (ChargingLoop0SH.IsPlaying()) ChargingLoop0SH.Stop();
        else {spdlog::debug("charging loop sound is already stopped."); soundState[SoundName::kChargingLoop] = State::kStopped;}
        soundState[SoundName::kChargingLoop] = State::kStopped;
    } else {
        chargingLoopStopUpdate.RegisterForUpdate(((float)a_delayMS) / 1000.f);
    }
}
void SoundManager::StopThrowingLoopSounds(const uint16_t a_delayMS)
{
    if (a_delayMS == 0) {
        if (ThrowingLoop0SH.IsPlaying()) ThrowingLoop0SH.Stop();
        else {spdlog::debug("throwing loop sound is already stopped."); soundState[SoundName::kThrowingLoop] = State::kStopped;}
        if (ThrowingLoop1SH.IsPlaying()) ThrowingLoop1SH.Stop();
        else {spdlog::debug("throwing loop sound is already stopped."); soundState[SoundName::kThrowingLoop] = State::kStopped;}
        soundState[SoundName::kThrowingLoop] = State::kStopped;
    } else {
        throwingLoopStopUpdate.RegisterForUpdate(((float)a_delayMS) / 1000.f);
    }
}
void SoundManager::StopArrivingLoopSounds(const uint16_t a_delayMS)
{
    if (a_delayMS == 0) {
        if (ArrivingLoop0SH.IsPlaying()) {ArrivingLoop0SH.Stop(); spdlog::debug("arriving loop sound 1 stopped.");}
        else {spdlog::debug("arriving loop sound 1 is already stopped."); soundState[SoundName::kArrivingLoop] = State::kStopped;}
        if (ArrivingLoop1SH.IsPlaying()) {ArrivingLoop1SH.Stop(); spdlog::debug("arriving loop sound 2 stopped.");}
        else {spdlog::debug("arriving loop sound 2 is already stopped."); soundState[SoundName::kArrivingLoop] = State::kStopped;}
        if (ArrivingLoop2SH.IsPlaying()) {ArrivingLoop2SH.Stop(); spdlog::debug("arriving loop sound 3 stopped.");}
        else {spdlog::debug("arriving loop sound 3 is already stopped."); soundState[SoundName::kArrivingLoop] = State::kStopped;}
        soundState[SoundName::kArrivingLoop] = State::kStopped;
    } else {
        arrivingLoopStopUpdate.RegisterForUpdate(((float)a_delayMS) / 1000.f);
    }
}
