#include "events.h"
#include "MainKratosCombat.h"
#include "util.h"

using namespace Util;

#pragma region AnimationEvents
bool AnimationEventTracker::Register()
{
    const auto pc = PlayerCharacter::GetSingleton();

    bool bSinked = false;
    bool bSuccess = pc->AddAnimationGraphEventSink(AnimationEventTracker::GetSingleton());
    if (bSuccess) {
        spdlog::info("Registered {}", typeid(BSAnimationGraphEvent).name());
    } else {
        BSAnimationGraphManagerPtr graphManager;
        pc->GetAnimationGraphManager(graphManager);
        if (graphManager) {         
            for (auto& animationGraph : graphManager->graphs) {
                if (bSinked) {
                    break;
                }
                auto eventSource = animationGraph->GetEventSource<BSAnimationGraphEvent>();
                for (auto& sink : eventSource->sinks) {
                    if (sink == AnimationEventTracker::GetSingleton()) {
                        bSinked = true;
                        break;
                    }
                }
            }
        }

        if (!bSinked) {
            spdlog::info("Failed to register {}", typeid(BSAnimationGraphEvent).name());
        }
    }
    return bSuccess || bSinked;
}
EventChecker AnimationEventTracker::ProcessEvent(const BSAnimationGraphEvent* a_event, BSTEventSource<BSAnimationGraphEvent>* a_eventSource)
{
    if (a_event) {
        std::string eventTag = a_event->tag.data();
        switch (hash(eventTag.data(), eventTag.size())) {
        // Start phase
        case "SkipNextEquipAnimation"_h:
            WeaponIdentify::skipEquipAnim = true;
            break;
//        case "BeginWeaponDraw"_h:
//            if (WeaponIdentify::isLeviathanAxe && Kratos::GetSingleton()->IsCanCallAxe()) {
//                if ((uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() > 1U && (uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() < 5U) LeviathanAxe::GetSingleton()->Call();
//            }
//            else if (WeaponIdentify::isMjolnir && Kratos::GetSingleton()->IsCanCallMjolnir()) {
//                if ((uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() > 1U && (uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() < 5U) Mjolnir::GetSingleton()->Call();
//            }
//            break;
        case "CallWeapon"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos) {
                switch (kratos->GetNextWeaponToCall())
                {
                case Kratos::Relic::kLeviathanAxe:
                    LeviathanAxe::GetSingleton()->Call();
                    break;
                case Kratos::Relic::kMjolnir:
                    Mjolnir::GetSingleton()->Call(false, false, Config::MjolnirArrivingDelay);
                    break;
#ifdef TRIDENT
                case Kratos::Relic::kTrident:
                    Trident::GetSingleton()->Call(10.f, 100.f, RE::PlayerCharacter::GetSingleton(), true);
                    break;
#endif
                default:
                    spdlog::warn("Can't found any weapon for ready to calling! Trying to call Levi");
                    LeviathanAxe::GetSingleton()->Call();
                    break;
                }
            }
/*
            if (WeaponIdentify::lastThrownRelic == Kratos::Relic::kLeviathanAxe) {
                if ((uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Mjolnir && (uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() > 1U) Mjolnir::GetSingleton()->Call();
#ifdef TRIDENT
                else if ((uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Trident && !Trident::GetSingleton()->isTridentThrowable) Trident::GetSingleton()->Call(10.f, 100.f, RE::PlayerCharacter::GetSingleton(), true);
#endif
                else LeviathanAxe::GetSingleton()->Call();
            }
            else if (WeaponIdentify::lastThrownRelic == Kratos::Relic::kMjolnir) {
                if ((uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::LeviathanAxe && (uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() > 1U) LeviathanAxe::GetSingleton()->Call();
#ifdef TRIDENT
                else if ((uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Trident && !Trident::GetSingleton()->isTridentThrowable) Trident::GetSingleton()->Call(10.f, 100.f, RE::PlayerCharacter::GetSingleton(), true);
#endif
                else Mjolnir::GetSingleton()->Call();
            }
#ifdef TRIDENT
            else if (WeaponIdentify::lastThrownRelic == Kratos::Relic::kTrident) {
                if (Trident::GetSingleton()->isTridentThrowable && WeaponIdentify::LeviathanAxe && (uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() > 1U) LeviathanAxe::GetSingleton()->Call();
                else if (Trident::GetSingleton()->isTridentThrowable && WeaponIdentify::Mjolnir && (uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() > 1U) Mjolnir::GetSingleton()->Call();
                else Trident::GetSingleton()->Call(10.f, 100.f, RE::PlayerCharacter::GetSingleton(), true);
            }
#endif
            else {spdlog::warn("Can't found any weapon for ready to calling! Trying to call Levi"); LeviathanAxe::GetSingleton()->Call();}
*/            break;
        case "CatchLevi"_h:
            break;
    //    case "LeviCallAttack"_h:     //event: attackPowerStartInPlace, attackStart, PowerAttack [IDLE:000E8456], NormalAttack [IDLE:00013215]
    //        if (auto Levi = LeviathanAxe::GetSingleton(); !WeaponIdentify::isLeviathanAxe && WeaponIdentify::LeviathanAxe) {
    //            Levi->Call(true);
    //            auto AnArchos = PlayerCharacter::GetSingleton();
    //            ObjectUtil::Actor::EquipItem(AnArchos, WeaponIdentify::LeviathanAxe, true, 1u, true, false, false, false);
    //            ResetEquipAnimationAfter(100, AnArchos);
    //        } else spdlog::info("Levi is not callable");
        case "ThrowAttackStart"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton()) {
                    Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag);
                }
            }
            else if (WeaponIdentify::isMjolnir) {
                if (auto mjolnir = Mjolnir::GetSingleton()) {
                    mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag);
                }
            }
        //    if (WeaponIdentify::isLeviathanAxe)
        //        Kratos::GetSingleton()->SetIsCanCharge(RE::PlayerCharacter::GetSingleton(), false);
        //    if (WeaponIdentify::isMjolnir)
        //        Kratos::GetSingleton()->SetIsCanCharge(RE::PlayerCharacter::GetSingleton(), false, Kratos::Relic::kMjolnir);
            break;
        case "ThrowWeapon"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton(); Levi->GetThrowState() == tState::kThrowable) {
                    Levi->Throw(false);
                } else spdlog::warn("Levi is not throwable");
            }
            if (WeaponIdentify::isMjolnir) {
                if (auto mjolnir = Mjolnir::GetSingleton(); mjolnir->GetThrowState() == tStateM::kThrowable) {
                    mjolnir->Throw(false);
                } else spdlog::warn("Mjolnir is not throwable");
            }
            if (WeaponIdentify::isDraupnirSpear) Draupnir::Throw();
            break;
        case "ThrowWeaponV"_h:
            if (auto Levi = LeviathanAxe::GetSingleton(); Levi && Levi->GetThrowState() == tState::kThrowable) {
                Levi->Throw(true);
            }
            else spdlog::warn("Levi is not throwable");
            break;
        case "ThrowWeaponH"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton(); Levi->GetThrowState() == tState::kThrowable) {
                    Levi->Throw(false, false, true);
                } else spdlog::warn("Levi is not throwable");
            }
            if (WeaponIdentify::isMjolnir) {
                if (auto mjolnir = Mjolnir::GetSingleton(); mjolnir->GetThrowState() == tStateM::kThrowable) {
                    mjolnir->Throw(false, false, true);
                } else spdlog::warn("Mjolnir is not throwable");
            }
            if (WeaponIdentify::isDraupnirSpear) Draupnir::Throw();
#ifdef TRIDENT
            else if(WeaponIdentify::isTrident) Trident::GetSingleton()->Throw();
#endif
            break;
        case "LeviChargeStart"_h:
            if (auto kratos = Kratos::GetSingleton(); auto AnArchos = RE::PlayerCharacter::GetSingleton()) {
                if (kratos && AnArchos) {
                    if (auto handEffect = kratos->VFXeffect.handFrost; handEffect)
                        AnArchos->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone, false);
                    if (auto soundEffect = kratos->soundEffect.chargeLevi; soundEffect)
                        ObjectUtil::Sound::PlaySound(soundEffect, WeaponIdentify::RHandBone, 5.f);
                }
            }
            break;
        case "LeviChargeEnd"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsCanCharge(PlayerCharacter::GetSingleton()))
                if (auto levi = LeviathanAxe::GetSingleton())
                    levi->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, -1);
            break;
        case "MjolnirChargeStart"_h:
            if (auto kratos = Kratos::GetSingleton(); auto AnArchos = RE::PlayerCharacter::GetSingleton()) {
                if (kratos && AnArchos) {
                    if (auto handEffect = kratos->VFXeffect.handShock; handEffect)
                        AnArchos->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone, false);
                    if (auto soundEffect = kratos->soundEffect.chargeLevi; soundEffect)
                        ObjectUtil::Sound::PlaySound(soundEffect, WeaponIdentify::RHandBone, 5.f);
                }
            }
            break;
        case "MjolnirCharge1"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsCanCharge(PlayerCharacter::GetSingleton(), Kratos::Relic::kMjolnir))
                if (auto mjolnir = Mjolnir::GetSingleton())
                    mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 1u, -1);
            break;
        case "MjolnirCharge2"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsCanCharge(PlayerCharacter::GetSingleton(), Kratos::Relic::kMjolnir))
                if (auto mjolnir = Mjolnir::GetSingleton())
                    mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 2u, -1);
            break;
        case "MjolnirCharge3"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsCanCharge(PlayerCharacter::GetSingleton(), Kratos::Relic::kMjolnir))
                if (auto mjolnir = Mjolnir::GetSingleton())
                    mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 3u, -1);
            break;
    //    case "MjolnirChargeEnd"_h:
    //        if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsCanCharge(PlayerCharacter::GetSingleton(), Kratos::Relic::kMjolnir))
    //            if (auto mjolnir = Mjolnir::GetSingleton())
    //                mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 3u, -1);
    //        break;
        case "ThrowSpear"_h:
            if (WeaponIdentify::isDraupnirSpear) Draupnir::Throw();
#ifdef TRIDENT
            else if(WeaponIdentify::isTrident) Trident::GetSingleton()->Throw();
#endif
            break;
        case "DraupnirsCallStage1"_h:
            if (WeaponIdentify::isDraupnirSpear) Draupnir::SetExplosionMagnitude(1.2f);
            break;
        case "DraupnirsCallStage2"_h:
            if (WeaponIdentify::isDraupnirSpear) Draupnir::SetExplosionMagnitude(1.5f);
            break;
        case "DraupnirsCall"_h:
            if (WeaponIdentify::isDraupnirSpear) Draupnir::Call(10.f, 100.f);
#ifdef TRIDENT
            else if (WeaponIdentify::isTrident || !Trident::GetSingleton()->isTridentThrowable) Trident::GetSingleton()->Call(10, 100);
#endif
            break;
        //  rage
        case "RageFuryTriggerStart"_h:
            Kratos::GetSingleton()->StartRage(Kratos::Rage::kFury);
            break;
        case "RageFuryTriggerEnd"_h:
    //        if (auto kratos = Kratos::GetSingleton(); kratos->IsInRage())
    //            kratos->SetIsCanRage(false);
            break;
        case "RageValorStart"_h:
            Kratos::GetSingleton()->StartRage(Kratos::Rage::kValor);
            break;
        case "RageValorEnd"_h:
            Kratos::GetSingleton()->EndRage(Kratos::Rage::kValor, true);
            break;
        case "RageFinish"_h:
            Kratos::GetSingleton()->EndRage(Kratos::GetSingleton()->GetLastTriggeredRageType(), true, false);
    //        Kratos::GetSingleton()->SetIsCanRage();
            break;
        case "weaponDraw"_h:
        //    WeaponIdentify::WeaponCheck();
            if (auto BoC = BladeOfChaos::GetSingleton()) {
                BoC->HideChains();
            }
            break;
        case "weaponSwing"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsInRage())
                kratos->RestoreRage(RE::PlayerCharacter::GetSingleton(), kratos->CalcRageDamageOrBuffAmount(360.f));
            break;
    //    case "CastOKStart"_h:
        case "MCO_AttackInitiate"_h:
        case "MCO_PowerAttackInitiate"_h:
        case "MCO_SprintAttackInitiate"_h:
        case "MCO_SprintPowerAttackInitiate"_h:
        case "Bfco_AttackStartFX"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton()) {
                    Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag);
                }
            }
            else if (WeaponIdentify::isMjolnir) {
                if (auto mjolnir = Mjolnir::GetSingleton()) {
                    mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag);
                }
            }
            break;
        case "AttackWinStart"_h:
        case "MCO_WinOpen"_h:
        case "MCO_PowerWinOpen"_h:
        case "BFCO_NextWinStart"_h:
        case "BFCO_NextPowerWinStart"_h:
        case "Collision_AttackEnd"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton()) {
                    Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, true);
                }
            }
            else if (WeaponIdentify::isMjolnir) {
                if (auto mjolnir = Mjolnir::GetSingleton()) {
                    mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag, true);
                }
            }
            break;
        case "InsertDraupnir"_h:
            Draupnir::MeleeThrow();
            break;
        case "RainOfSpear"_h:
            if (WeaponIdentify::isDraupnirSpear) Draupnir::ArtilleryOfTheAncients(0.1f, 3.f);
#ifdef TRIDENT
            else if(WeaponIdentify::isTrident) Trident::GetSingleton()->TrishulsMight(1.f, 6.f);
#endif
            break;
        case "chainOpenR"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
                BoC->HideChains(false);
            }
            break;
        case "chainOpenL"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
                BoC->HideChains(false);
            }
            break;
        case "FlameWhiplashStart"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
            //    if (!BoC->IsScorching()) RE::PlayerCharacter::GetSingleton()->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kSpeedMult, 0.8f);
                BoC->SetIsScorching();
                BoC->SetScorchingSpeed(0.5f);
            }
            break;
        case "FlameWhiplashLoop"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
                if (BoC->IsQueueEnd()) {
                    RE::PlayerCharacter::GetSingleton()->NotifyAnimationGraph("chainCloseR");
                    RE::PlayerCharacter::GetSingleton()->NotifyAnimationGraph("IdleStop");
                }
            }
            break;
        case "FlameWhiplashEnd"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
            //    if (!BoC->IsScorching()) RE::PlayerCharacter::GetSingleton()->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kSpeedMult, -0.8f);
                BoC->SetIsScorching(false);
            }
            break;
    //    case "BFCO_DIY_recovery"_h:
    //    case "MCO_Recovery"_h:
        case "MCO_AttackStateExit"_h:
        case "tailCombatState"_h:
        case "tailCombatIdle"_h:
        case "attackStop"_h:
        case "IdleStop"_h:
        case "CastOKStop"_h:
            if (WeaponIdentify::unequipWhenAnimEnds) {
                if (auto AnArchos = PlayerCharacter::GetSingleton(); AnArchos) {
                    ObjectUtil::Actor::UnEquipItem(AnArchos, false, false, true, true, WeaponIdentify::skipEquipAnim, false);
                    ObjectUtil::Actor::ResetEquipAnimationAfter(100, AnArchos);
                } WeaponIdentify::unequipWhenAnimEnds = false;
            }
#ifdef EXPERIMENTAL_SHIELD
            //  animated shield
            ObjectUtil::Actor::SendAnimationEvent(PlayerCharacter::GetSingleton(), "shieldClose");
#endif
            break;
            if (auto BoC = BladeOfChaos::GetSingleton()) {
                BoC->HideChains(true);
            }
        case "throwAttackReady"_h:
        case "throwPowerAttackReady"_h:
            if (Config::IsAdvancedThrowingInstalled) {
                if (auto AnArchos = PlayerCharacter::GetSingleton(); AnArchos && WeaponIdentify::isRelic) {
                    bool isThrowing; AnArchos->GetGraphVariableBool("bIsThrowing", isThrowing);
                    if (isThrowing) {
                        bool isChargingThrow; AnArchos->GetGraphVariableBool("bIsPressingAttackButton", isChargingThrow);
                        if (!isChargingThrow) AnArchos->GetGraphVariableBool("bIsPressingPowerAttackButton", isChargingThrow);
                        if (auto kratos = Kratos::GetSingleton(); kratos) {
                            if (auto Levi = LeviathanAxe::GetSingleton(); WeaponIdentify::isLeviathanAxe && Levi->GetThrowState() == tState::kThrowable) {
                                Levi->data.throwingChargeDuration = 0.f;
                                if (isChargingThrow) Levi->StartChargingThrow(AnArchos);
                            } else if (auto mjolnir = Mjolnir::GetSingleton(); WeaponIdentify::isMjolnir && mjolnir->GetThrowState() == tStateM::kThrowable) {
                                mjolnir->data.throwingChargeDuration = 0.f;
                                if (isChargingThrow) mjolnir->StartChargingThrow(AnArchos);
                            } else if (WeaponIdentify::isDraupnirSpear) {
                                Draupnir::data.throwingChargeDuration = 0.f;
                                if (isChargingThrow) Draupnir::StartChargingThrow(AnArchos);
#ifdef TRIDENT
                            } else if (WeaponIdentify::isTrident) {
                                if (isChargingThrow) Trident::StartChargingThrow(AnArchos);
#endif
                            }
                        }
                    }
                }
            }
            break;
        case "throwAttackEndStart"_h:
        case "throwPowerAttackEndStart"_h:
            if (Config::IsAdvancedThrowingInstalled) {
                if (auto kratos = Kratos::GetSingleton(); kratos) kratos->SetIsChargingThrow(false);
            }
            break;
        }
    }
        return EventChecker::kContinue;
}
bool AnimObjectAnimationEventTracker::Register()
{
    const auto pc = PlayerCharacter::GetSingleton();

    bool bSinked = false;
    bool bSuccess = pc->AddAnimationGraphEventSink(AnimObjectAnimationEventTracker::GetSingleton());
    if (bSuccess) {
        spdlog::info("Registered {}", typeid(AnimObjectAnimationEventTracker).name());
    } else {
        BSAnimationGraphManagerPtr graphManager;
        pc->GetBiped1(false)->objects->weaponManager->GetAnimationGraphManager(graphManager);
        if (graphManager) {
            for (auto& animationGraph : graphManager->graphs) {
                if (bSinked) {
                    break;
                }
                auto eventSource = animationGraph->GetEventSource<BSAnimationGraphEvent>();
                for (auto& sink : eventSource->sinks) {
                    if (sink == AnimObjectAnimationEventTracker::GetSingleton()) {
                        bSinked = true;
                        break;
                    }
                }
            }
        }

        if (!bSinked) {
            spdlog::info("Failed to register {}", typeid(AnimObjectAnimationEventTracker).name());
        }

    }
    return bSuccess || bSinked;
}
EventChecker AnimObjectAnimationEventTracker::ProcessEvent(const BSAnimationGraphEvent* a_event, BSTEventSource<BSAnimationGraphEvent>* a_eventSource)
{
    if (a_event) {
        std::string eventTag = a_event->tag.data();
        switch (hash(eventTag.data(), eventTag.size())) {
        case "chainClosedR"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
                BoC->HideChains();
            }
            break;
        case "chainClosedL"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
                BoC->HideChains();
            }
            break;
        }
    }
        return EventChecker::kContinue;
}
#pragma endregion
bool MagicEffectApplyTracker::Register()
{
    auto sourceHolder = RE::ScriptEventSourceHolder::GetSingleton(); 
    if (sourceHolder) {
        sourceHolder->AddEventSink(MagicEffectApplyTracker::GetSingleton());
            spdlog::info("Magic effect apply event sink registered!");
            return true;
    } else  spdlog::warn("Magic effect apply event sink not registered!");
    return false;
}
EventChecker MagicEffectApplyTracker::ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>* a_eventSource)
{
    if (a_event) {
        auto formID = a_event->magicEffect;
        auto casterRef = a_event->caster.get();
        auto targetRef = a_event->target.get();
        if (casterRef && targetRef && casterRef == targetRef) {
            auto caster = casterRef->As<RE::Actor>();
            auto casterMT = caster ? caster->AsMagicTarget() : nullptr;
            auto kratos = Kratos::GetSingleton();
            auto levi = LeviathanAxe::GetSingleton();
            auto mjolnir = Mjolnir::GetSingleton();
            if (!casterMT || !kratos || !levi || !mjolnir) {spdlog::warn("caster magic target or kratos or levi or mjolnir does not exists!"); return EventChecker::kContinue;}

            if (formID == kratos->spellID.aim) {
                spdlog::debug("aiming...");
            } else if (formID == kratos->spellID.call) {
                if (!WeaponIdentify::isRelic) {
                    if (levi->data.weap || mjolnir->data.weap || WeaponIdentify::Trident) {
                        if ((uint_fast8_t)levi->GetThrowState() <= 3U && (uint_fast8_t)levi->GetThrowState() != 0U) {
                            caster->SetGraphVariableInt("iNextWeaponToCall", (uint32_t)kratos->GetNextWeaponToCall());
                            caster->SetGraphVariableBool("bLeviInCatchRange", false);
                            kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                        } else if ((uint_fast8_t)mjolnir->GetThrowState() <= 3U && (uint_fast8_t)mjolnir->GetThrowState() != 0U) {
                            caster->SetGraphVariableInt("iNextWeaponToCall", (uint32_t)kratos->GetNextWeaponToCall());
                            caster->SetGraphVariableBool("bLeviInCatchRange", false);
                            kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
#ifdef TRIDENT
                        } else if (!Trident::GetSingleton()->isTridentThrowable) {
                            caster->SetGraphVariableInt("iNextWeaponToCall", (uint32_t)kratos->GetNextWeaponToCall());
                            caster->SetGraphVariableBool("bLeviInCatchRange", false);
                            kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
#endif
                        } else spdlog::info("levi and mjolnir can't arrive!");
                    } else spdlog::info("levi and mjolnir does not exist!");
                } else if (WeaponIdentify::isLeviathanAxe) {
                    if (!levi->isAxeThrowed && kratos->IsCanCharge(caster, Kratos::Relic::kLeviathanAxe)) {
                        kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                    }
                } else if (WeaponIdentify::isBladeOfChaos) {
                    BladeOfChaos::GetSingleton()->Update(*g_engineTime);
                    BladeOfChaos::GetSingleton()->BuffScorchingSpeed();
                    kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                } else if (WeaponIdentify::isDraupnirSpear || WeaponIdentify::isTrident) {
                    kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                } else if (WeaponIdentify::isMjolnir) {
                    if (!mjolnir->isMjolnirThrowed && kratos->IsCanCharge(caster, Kratos::Relic::kMjolnir)) {
                        kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                    }
                }
            } else if (formID == kratos->spellID.runic) {
                if (caster->HasSpell(kratos->SpellFinisherButton)) {
                    kratos->DoKratosAction(Kratos::Action::kRage, caster);
                }
            } else if (formID == kratos->spellID.finisher) {
                if (caster->HasSpell(kratos->SpellRunicButton)) {
                    kratos->DoKratosAction(Kratos::Action::kRage, caster);
                }
            } else if (formID == kratos->spellID.leviChargeCoolDown) {
                spdlog::debug("levi charge in cooldown...");
            }
        }
    }   return EventChecker::kContinue;
}

bool InputEventTracker::Register()
{
    auto sourceHolder = RE::BSInputDeviceManager::GetSingleton(); 
    if (sourceHolder) {
        sourceHolder->AddEventSink(InputEventTracker::GetSingleton());
            spdlog::info("input event sink registered!");
            return true;
    } else  spdlog::warn("input event sink not registered!");
    return false;
};
std::uint32_t InputEventTracker::GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key)
{
    using Key = RE::BSWin32GamepadDevice::Key;

    std::uint32_t index;
    switch (a_key) 
    {
    case Key::kUp:
        index = 0;
        break;
    case Key::kDown:
        index = 1;
        break;
    case Key::kLeft:
        index = 2;
        break;
    case Key::kRight:
        index = 3;
        break;
    case Key::kStart:
        index = 4;
        break;
    case Key::kBack:
        index = 5;
        break;
    case Key::kLeftThumb:
        index = 6;
        break;
    case Key::kRightThumb:
        index = 7;
        break;
    case Key::kLeftShoulder:
        index = 8;
        break;
    case Key::kRightShoulder:
        index = 9;
        break;
    case Key::kA:
        index = 10;
        break;
    case Key::kB:
        index = 11;
        break;
    case Key::kX:
        index = 12;
        break;
    case Key::kY:
        index = 13;
        break;
    case Key::kLeftTrigger:
        index = 14;
        break;
    case Key::kRightTrigger:
        index = 15;
        break;
    default:
        index = kInvalid;
        break;
    } return index != kInvalid ? index + kGamepadOffset : kInvalid;
}
std::uint32_t InputEventTracker::GetOffsettedKeyCode(std::uint32_t a_keyCode, RE::INPUT_DEVICE a_inputDevice) const
{
    switch (a_inputDevice) {
    case RE::INPUT_DEVICE::kKeyboard:
        break;
    case RE::INPUT_DEVICE::kMouse:
        a_keyCode += kMouseOffset;
        break;
    case RE::INPUT_DEVICE::kGamepad:
        a_keyCode = GetGamepadIndex((RE::BSWin32GamepadDevice::Key)a_keyCode);
        break;
    default:
        break;
    } return a_keyCode;
}
EventChecker InputEventTracker::ProcessEvent(RE::InputEvent* const *a_event, RE::BSTEventSource<RE::InputEvent*> *a_eventSource)
{
    if (!a_event || RE::UI::GetSingleton()->GameIsPaused()) return EventChecker::kContinue;

    for (auto event = *a_event; event; event = event->next) {
        if (!event->HasIDCode() || event->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) continue;

        auto keyCode = event->AsIDEvent()->GetIDCode();

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return EventChecker::kContinue;

        auto kratos = Kratos::GetSingleton();
        if (!kratos) return EventChecker::kContinue;

        if (auto button = static_cast<RE::ButtonEvent*>(event); button) {
            auto device = event->device.get();
            keyCode = GetOffsettedKeyCode(keyCode, device);
            if (keyCode == Config::AxeCallKey) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellAxeCallButton); player->SetGraphVariableBool("bPressingCallButton", true);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellAxeCallButton); player->SetGraphVariableBool("bPressingCallButton", false);}
            }
            else if (keyCode == Config::AimKey) {
                if (button->IsDown()) {kratos->Aim(true); player->AddSpell(kratos->SpellAimButton);/* player->SetGraphVariableBool("bIsAiming", true);*/}
                else if (button->IsUp()) {kratos->Aim(false); player->RemoveSpell(kratos->SpellAimButton);/* player->SetGraphVariableBool("bIsAiming", false);*/}
            }
            else if (keyCode == Config::RunicKey) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellRunicButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellRunicButton);}
            }
            else if (keyCode == Config::FinisherKey) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellFinisherButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellFinisherButton);}
            }
            else if (keyCode == Config::MediumDistanceKey) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellMidDistButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellMidDistButton);}
            }
            else if (keyCode == Config::LongDistanceKey) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellLongDistButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellLongDistButton);}
            }
        }
    } return EventChecker::kContinue;
}
bool MenuOpenCloseTracker::Register()
{
    auto sourceHolder = RE::UI::GetSingleton(); 
    if (sourceHolder) {
        sourceHolder->AddEventSink(MenuOpenCloseTracker::GetSingleton());
            spdlog::info("menu open close event sink registered!");
            return true;
    } else  spdlog::warn("menu open close event sink not registered!");
    return false;
};
EventChecker MenuOpenCloseTracker::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource)
{
    if (a_event) {
        if (a_event->opening) {
            auto Levi = LeviathanAxe::GetSingleton();
            Levi->soundData.PauseAllLoopingSounds();
            auto mjolnir = Mjolnir::GetSingleton();
            mjolnir->soundData.PauseAllLoopingSounds();
        } else {
            auto Levi = LeviathanAxe::GetSingleton();
            Levi->soundData.ContinueAllLoopingSounds();
            auto mjolnir = Mjolnir::GetSingleton();
            mjolnir->soundData.ContinueAllLoopingSounds();
        }
    } return EventChecker::kContinue;
}