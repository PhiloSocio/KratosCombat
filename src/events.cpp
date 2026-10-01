#include "events.h"
#include "util.h"

#ifdef KRATOS_COMBAT_3
    #include "RelicManager.h"
#else
    #include "MainKratosCombat.h"
    using RageType = Kratos::Rage;
    using RelicType = Kratos::Relic;
    using ActionType = Kratos::Action;
#endif

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

#ifdef KRATOS_COMBAT_3
EventChecker AnimationEventTracker::ProcessEvent(const BSAnimationGraphEvent* a_event, BSTEventSource<BSAnimationGraphEvent>* a_eventSource)
{
    if (a_event) {

        auto manager = RelicManager::GetSingleton();
        auto player = manager ? manager->GetPlayer() : nullptr;
        if (!player) {spdlog::warn("kratos player does not exists!"); return EventChecker::kContinue;}

        std::string eventTag = a_event->tag.data();
        switch (hash(eventTag.data(), eventTag.size())) {
        // Start phase
        case "SkipNextEquipAnimation"_h:
            player->SetSkipEquipAnim(true);
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
            player->CallWeapon();
        case "CatchLevi"_h:
            break;
        case "ThrowAttackStart"_h:
            if (auto rHandRelic = player->GetRightHandRelic())
                rHandRelic->ResetCharge(rHandRelic->enchMag, rHandRelic->defaultEnchMag);
            break;
        case "ThrowWeapon"_h:
            player->ThrowWeapon(RotationType::kNone, ThrowType::kNormalThrow);
            break;
        case "ThrowWeaponL"_h:
            player->ThrowWeapon(RotationType::kSpinLateral, ThrowType::kNormalThrow);
            break;
        case "ThrowWeaponV"_h:
            player->ThrowWeapon(RotationType::kSpinVertical, ThrowType::kPowerThrow);
            break;
        case "ThrowWeaponH"_h:
            player->ThrowWeapon(RotationType::kSpinLateral, ThrowType::kHomingThrow);
            break;
        case "LeviChargeStart"_h:
        //    player->ChargeWeaponStart();
            break;
        case "LeviChargeEnd"_h:
        //    player->ChargeWeaponEnd();
    //        if (auto manager = Kratos::GetSingleton(); manager && manager->IsCanCharge(PlayerCharacter::GetSingleton()))
    //            if (auto levi = LeviathanAxe::GetSingleton())
    //                levi->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, -1);
            break;
        case "MjolnirChargeStart"_h:
        //    player->ChargeWeaponStart();
            break;
        case "MjolnirCharge1"_h:
        //    player->ChargeWeapon();
    //        if (auto manager = Kratos::GetSingleton(); manager && manager->IsCanCharge(PlayerCharacter::GetSingleton(), RelicType::kMjolnir))
    //            if (auto mjolnir = Mjolnir::GetSingleton())
    //                mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 1u, -1);
            break;
        case "MjolnirCharge2"_h:
        //    player->ChargeWeapon();
    //        if (auto manager = Kratos::GetSingleton(); manager && manager->IsCanCharge(PlayerCharacter::GetSingleton(), RelicType::kMjolnir))
    //            if (auto mjolnir = Mjolnir::GetSingleton())
    //                mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 2u, -1);
            break;
        case "MjolnirCharge3"_h:
        //    player->ChargeWeapon();
    //        if (auto manager = Kratos::GetSingleton(); manager && manager->IsCanCharge(PlayerCharacter::GetSingleton(), RelicType::kMjolnir))
    //            if (auto mjolnir = Mjolnir::GetSingleton())
    //                mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 3u, -1);
            break;
//        case "MjolnirChargeEnd"_h:
        //    player->ChargeEnd();
    //        if (auto manager = Kratos::GetSingleton(); manager && manager->IsCanCharge(PlayerCharacter::GetSingleton(), RelicType::kMjolnir))
    //            if (auto mjolnir = Mjolnir::GetSingleton())
    //                mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 3u, -1);
    //        break;
        case "ThrowSpear"_h:
            player->ThrowWeapon(RotationType::kNone, ThrowType::kNormalThrow);
            break;
        case "DraupnirsCallStage1"_h:
//            if (WeaponIdentify::isDraupnirSpear) Draupnir::SetExplosionMagnitude(1.2f);
            break;
        case "DraupnirsCallStage2"_h:
//            if (WeaponIdentify::isDraupnirSpear) Draupnir::SetExplosionMagnitude(1.5f);
            break;
        case "DraupnirsCall"_h:
//            if (WeaponIdentify::isDraupnirSpear) Draupnir::Call(10.f, 100.f);
#ifdef TRIDENT
            else if (WeaponIdentify::isTrident || !Trident::GetSingleton()->isTridentThrowable) Trident::GetSingleton()->Call(10, 100);
#endif
            break;
        //  rage
        case "RageFuryTriggerStart"_h:
            player->SetRageType(RageType::kFury);
            player->StartRage();
            break;
        case "RageFuryTriggerEnd"_h:
            break;
        case "RageValorStart"_h:
            player->SetRageType(RageType::kValor);
            player->StartRage();
            break;
        case "RageValorEnd"_h:
            player->EndRage(true);
            break;
        case "RageFinish"_h:
            player->EndRage(true);
            break;
        case "weaponDraw"_h:
        //    player->WeaponDrawEvent();
            break;
        case "weaponSwing"_h:
            player->RestoreRage(player->CalcRageDamageOrBuffAmount(360), true);
            break;
    //    case "CastOKStart"_h:
        case "MCO_AttackInitiate"_h:
        case "MCO_PowerAttackInitiate"_h:
        case "MCO_SprintAttackInitiate"_h:
        case "MCO_SprintPowerAttackInitiate"_h:
        case "Bfco_AttackStartFX"_h:
        //    player->OnAttackStart();
    //        if (auto manager = Kratos::GetSingleton(); manager && manager->IsInRage())
    //            manager->RestoreRage(RE::PlayerCharacter::GetSingleton(), -*manager->values.rageDamageAmount * 0.25f, false);
    //        if (WeaponIdentify::isLeviathanAxe) {
    //            if (auto Levi = LeviathanAxe::GetSingleton()) {
    //                Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag);
    //            }
    //        }
    //        else if (WeaponIdentify::isMjolnir) {
    //            if (auto mjolnir = Mjolnir::GetSingleton()) {
    //                mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag);
    //            }
    //        }
            break;
        case "AttackWinStart"_h:
        case "MCO_WinOpen"_h:
        case "MCO_PowerWinOpen"_h:
        case "BFCO_NextWinStart"_h:
        case "BFCO_NextPowerWinStart"_h:
        case "Collision_AttackEnd"_h:
        //    player->OnAttackEnd();
    //        if (auto manager = Kratos::GetSingleton(); manager && manager->IsInRage())
    //            manager->RestoreRage(RE::PlayerCharacter::GetSingleton(), -*manager->values.rageDamageAmount * 0.25f, false);
    //        if (WeaponIdentify::isLeviathanAxe) {
    //            if (auto Levi = LeviathanAxe::GetSingleton()) {
    //                Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, true);
    //            }
    //        }
    //        else if (WeaponIdentify::isMjolnir) {
    //            if (auto mjolnir = Mjolnir::GetSingleton()) {
    //                mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag, true);
    //            }
    //        }
            break;
        case "InsertDraupnir"_h:
            player->ThrowWeapon(RotationType::kNone, ThrowType::kMelee);
            break;
        case "RainOfSpear"_h:
//            if (WeaponIdentify::isDraupnirSpear) Draupnir::ArtilleryOfTheAncients(0.1f, 3.f);
#ifdef TRIDENT
            else if(WeaponIdentify::isTrident) Trident::GetSingleton()->TrishulsMight(1.f, 6.f);
#endif
            break;
        case "chainOpenR"_h:
    //        if (auto BoC = BladeOfChaos::GetSingleton()) {
    //            BoC->HideChains(false);
    //        }
            break;
        case "chainOpenL"_h:
    //        if (auto BoC = BladeOfChaos::GetSingleton()) {
    //            BoC->HideChains(false);
    //        }
            break;
        case "FlameWhiplashStart"_h:
        //    player->ChargeWeaponStart();
    //        if (auto BoC = BladeOfChaos::GetSingleton()) {
    //        //    if (!BoC->IsScorching()) RE::PlayerCharacter::GetSingleton()->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kSpeedMult, 0.8f);
    //            BoC->SetIsScorching();
    //            BoC->SetScorchingSpeed(0.5f);
    //        }
            break;
        case "FlameWhiplashLoop"_h:
    //        if (auto BoC = BladeOfChaos::GetSingleton()) {
    //            if (BoC->IsQueueEnd()) {
    //                RE::PlayerCharacter::GetSingleton()->NotifyAnimationGraph("chainCloseR");
    //                RE::PlayerCharacter::GetSingleton()->NotifyAnimationGraph("IdleStop");
    //            }
    //        }
            break;
        case "FlameWhiplashEnd"_h:
    //        if (auto BoC = BladeOfChaos::GetSingleton()) {
    //        //    if (!BoC->IsScorching()) RE::PlayerCharacter::GetSingleton()->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kSpeedMult, -0.8f);
    //            BoC->SetIsScorching(false);
    //        }
            break;
//        case "BFCO_DIY_recovery"_h:
//        case "MCO_Recovery"_h:
        case "MCO_AttackStateExit"_h:
        case "tailCombatState"_h:
        case "tailCombatIdle"_h:
        case "attackStop"_h:
        case "IdleStop"_h:
        case "CastOKStop"_h:
        //    player->OnCombatReadyStateStart();
//            if (auto manager = Kratos::GetSingleton(); manager && manager->IsInRage())
//                manager->RestoreRage(RE::PlayerCharacter::GetSingleton(), -*manager->values.rageDamageAmount * 0.25f, false);
//            if (WeaponIdentify::unequipWhenAnimEnds) {
//                if (auto AnArchos = PlayerCharacter::GetSingleton(); AnArchos) {
//                    ObjectUtil::Actor::UnEquipItem(AnArchos, false, false, true, true, WeaponIdentify::skipEquipAnim, false);
//                    ObjectUtil::Actor::ResetEquipAnimationAfter(100, AnArchos);
//                } WeaponIdentify::unequipWhenAnimEnds = false;
//            }
#ifdef EXPERIMENTAL_SHIELD
            //  animated shield
            ObjectUtil::Actor::SendAnimationEvent(PlayerCharacter::GetSingleton(), "shieldClose");
#endif
            break;
//            if (auto BoC = BladeOfChaos::GetSingleton()) {
//                BoC->HideChains(true);
//            }
        case "throwAttackReady"_h:
        case "throwPowerAttackReady"_h:
            if (Config::IsAdvancedThrowingInstalled) {
                player->StartChargingThrow();
            }
            break;
        case "throwAttackEndStart"_h:
        case "throwPowerAttackEndStart"_h:
            if (Config::IsAdvancedThrowingInstalled) {
                player->StopChargingThrow();
            }
            break;
        case "FootLeft"_h:
        case "FootRight"_h:
        case "PickNewIdle"_h:
            if (player->IsInRage())
                player->RestoreRage(player->GetRageDamageAmount() * 0.25f, false);
            break;
        }
    }
        return EventChecker::kContinue;
}
#else
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
            if (auto manager = Kratos::GetSingleton(); manager) {
                switch (manager->GetNextWeaponToCall())
                {
                case RelicType::kLeviathanAxe:
                    LeviathanAxe::GetSingleton()->Call();
                    break;
                case RelicType::kMjolnir:
                    Mjolnir::GetSingleton()->Call(false, false, Config::MjolnirArrivingDelay);
                    break;
#ifdef TRIDENT
                case RelicType::kTrident:
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
            if (WeaponIdentify::lastThrownRelic == RelicType::kLeviathanAxe) {
                if ((uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Mjolnir && (uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() > 1U) Mjolnir::GetSingleton()->Call();
#ifdef TRIDENT
                else if ((uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Trident && !Trident::GetSingleton()->isTridentThrowable) Trident::GetSingleton()->Call(10.f, 100.f, RE::PlayerCharacter::GetSingleton(), true);
#endif
                else LeviathanAxe::GetSingleton()->Call();
            }
            else if (WeaponIdentify::lastThrownRelic == RelicType::kMjolnir) {
                if ((uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::LeviathanAxe && (uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() > 1U) LeviathanAxe::GetSingleton()->Call();
#ifdef TRIDENT
                else if ((uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Trident && !Trident::GetSingleton()->isTridentThrowable) Trident::GetSingleton()->Call(10.f, 100.f, RE::PlayerCharacter::GetSingleton(), true);
#endif
                else Mjolnir::GetSingleton()->Call();
            }
#ifdef TRIDENT
            else if (WeaponIdentify::lastThrownRelic == RelicType::kTrident) {
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
        //        Kratos::GetSingleton()->SetIsCanCharge(RE::PlayerCharacter::GetSingleton(), false, RelicType::kMjolnir);
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
            if (auto manager = Kratos::GetSingleton(); auto AnArchos = RE::PlayerCharacter::GetSingleton()) {
                if (manager && AnArchos) {
                    if (auto handEffect = manager->VFXeffect.handFrost; handEffect)
                        AnArchos->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone, false);
                    if (auto soundEffect = manager->soundEffect.chargeLevi; soundEffect)
                        ObjectUtil::Sound::PlaySound(soundEffect, WeaponIdentify::RHandBone, 5.f);
                }
            }
            break;
        case "LeviChargeEnd"_h:
            if (auto manager = Kratos::GetSingleton(); manager && manager->IsCanCharge(PlayerCharacter::GetSingleton()))
                if (auto levi = LeviathanAxe::GetSingleton())
                    levi->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, -1);
            break;
        case "MjolnirChargeStart"_h:
            if (auto manager = Kratos::GetSingleton(); auto AnArchos = RE::PlayerCharacter::GetSingleton()) {
                if (manager && AnArchos) {
                    if (auto handEffect = manager->VFXeffect.handShock; handEffect)
                        AnArchos->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone, false);
                    if (auto soundEffect = manager->soundEffect.chargeLevi; soundEffect)
                        ObjectUtil::Sound::PlaySound(soundEffect, WeaponIdentify::RHandBone, 5.f);
                }
            }
            break;
        case "MjolnirCharge1"_h:
            if (auto manager = Kratos::GetSingleton(); manager && manager->IsCanCharge(PlayerCharacter::GetSingleton(), RelicType::kMjolnir))
                if (auto mjolnir = Mjolnir::GetSingleton())
                    mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 1u, -1);
            break;
        case "MjolnirCharge2"_h:
            if (auto manager = Kratos::GetSingleton(); manager && manager->IsCanCharge(PlayerCharacter::GetSingleton(), RelicType::kMjolnir))
                if (auto mjolnir = Mjolnir::GetSingleton())
                    mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 2u, -1);
            break;
        case "MjolnirCharge3"_h:
            if (auto manager = Kratos::GetSingleton(); manager && manager->IsCanCharge(PlayerCharacter::GetSingleton(), RelicType::kMjolnir))
                if (auto mjolnir = Mjolnir::GetSingleton())
                    mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 3u, -1);
            break;
    //    case "MjolnirChargeEnd"_h:
    //        if (auto manager = Kratos::GetSingleton(); manager && manager->IsCanCharge(PlayerCharacter::GetSingleton(), RelicType::kMjolnir))
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
            Kratos::GetSingleton()->StartRage(RageType::kFury);
            break;
        case "RageFuryTriggerEnd"_h:
    //        if (auto manager = Kratos::GetSingleton(); manager->IsInRage())
    //            manager->SetIsCanRage(false);
            break;
        case "RageValorStart"_h:
            Kratos::GetSingleton()->StartRage(RageType::kValor);
            break;
        case "RageValorEnd"_h:
            Kratos::GetSingleton()->EndRage(RageType::kValor, true);
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
            if (auto manager = Kratos::GetSingleton(); manager && manager->IsInRage())
                manager->RestoreRage(RE::PlayerCharacter::GetSingleton(), manager->CalcRageDamageOrBuffAmount(360.f), true);
            break;
    //    case "CastOKStart"_h:
        case "MCO_AttackInitiate"_h:
        case "MCO_PowerAttackInitiate"_h:
        case "MCO_SprintAttackInitiate"_h:
        case "MCO_SprintPowerAttackInitiate"_h:
        case "Bfco_AttackStartFX"_h:
            if (auto manager = Kratos::GetSingleton(); manager && manager->IsInRage())
                manager->RestoreRage(RE::PlayerCharacter::GetSingleton(), -*manager->values.rageDamageAmount * 0.25f, false);
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
            if (auto manager = Kratos::GetSingleton(); manager && manager->IsInRage())
                manager->RestoreRage(RE::PlayerCharacter::GetSingleton(), -*manager->values.rageDamageAmount * 0.25f, false);
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
            if (auto manager = Kratos::GetSingleton(); manager && manager->IsInRage())
                manager->RestoreRage(RE::PlayerCharacter::GetSingleton(), -*manager->values.rageDamageAmount * 0.25f, false);
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
                        if (auto manager = Kratos::GetSingleton(); manager) {
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
                if (auto manager = Kratos::GetSingleton(); manager) manager->SetIsChargingThrow(false);
            }
            break;
        case "FootLeft"_h:
        case "FootRight"_h:
        case "PickNewIdle"_h:
            if (auto manager = Kratos::GetSingleton(); manager && manager->IsInRage())
                manager->RestoreRage(RE::PlayerCharacter::GetSingleton(), -*manager->values.rageDamageAmount * 0.25f, false);
            break;
        }
    }
        return EventChecker::kContinue;
}
#endif
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
#ifdef KRATOS_COMBAT_3
EventChecker AnimObjectAnimationEventTracker::ProcessEvent(const BSAnimationGraphEvent* a_event, BSTEventSource<BSAnimationGraphEvent>* a_eventSource)
{
    if (a_event) {
        std::string eventTag = a_event->tag.data();
        switch (hash(eventTag.data(), eventTag.size())) {
        case "chainClosedR"_h:
            break;
        case "chainClosedL"_h:
            break;
        }
    }
        return EventChecker::kContinue;
}
#else
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
#endif
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
        const auto formID = a_event->magicEffect;
        auto casterRef = a_event->caster.get();
        auto targetRef = a_event->target.get();
        if (casterRef && targetRef && casterRef == targetRef) {
            auto caster = casterRef->As<RE::Actor>();

#ifdef KRATOS_COMBAT_3
            auto assets = Assets::GetSingleton();
            auto manager = RelicManager::GetSingleton();
            auto player = manager ? manager->GetPlayer() : nullptr;
            if (!player) {spdlog::warn("kratos player does not exists!"); return EventChecker::kContinue;}
#else
            auto manager = Kratos::GetSingleton();
            auto levi = LeviathanAxe::GetSingleton();
            auto mjolnir = Mjolnir::GetSingleton();
            if (!manager || !levi || !mjolnir) {spdlog::warn("cmanager or levi or mjolnir does not exists!"); return EventChecker::kContinue;}
#endif

#ifdef KRATOS_COMBAT_3
            if (formID == assets->spellID.call) {
                if (!player->GetRightHandRelic()) {
                    player->DoAction(ActionType::kWeaponCall);;
                } else {
                    player->DoAction(ActionType::kWeaponCharge);;
                }
            } else if (formID == assets->spellID.runic) {
                if (caster->HasSpell(assets->SpellFinisherButton)) {
                    player->StartRage(true);
                }
            } else if (formID == assets->spellID.finisher) {
                if (caster->HasSpell(assets->SpellRunicButton)) {
                    player->StartRage(true);
                }
            } else if (formID == assets->spellID.leviChargeCoolDown) {
                spdlog::debug("levi charge in cooldown...");
            }
#else
            if (formID == manager->spellID.aim) {
                spdlog::debug("aiming...");
            } else if (formID == manager->spellID.call) {
                if (!WeaponIdentify::isRelic) {
                    if (levi->data.weap || mjolnir->data.weap || WeaponIdentify::Trident) {
                        if ((uint_fast8_t)levi->GetThrowState() <= 3U && (uint_fast8_t)levi->GetThrowState() != 0U) {
                            caster->SetGraphVariableInt("iNextWeaponToCall", (uint32_t)manager->GetNextWeaponToCall());
                            caster->SetGraphVariableBool("bLeviInCatchRange", false);
                            manager->DoKratosAction(ActionType::kWeaponCharge, caster);
                        } else if ((uint_fast8_t)mjolnir->GetThrowState() <= 3U && (uint_fast8_t)mjolnir->GetThrowState() != 0U) {
                            caster->SetGraphVariableInt("iNextWeaponToCall", (uint32_t)manager->GetNextWeaponToCall());
                            caster->SetGraphVariableBool("bLeviInCatchRange", false);
                            manager->DoKratosAction(ActionType::kWeaponCharge, caster);
#ifdef TRIDENT
                        } else if (!Trident::GetSingleton()->isTridentThrowable) {
                            caster->SetGraphVariableInt("iNextWeaponToCall", (uint32_t)manager->GetNextWeaponToCall());
                            caster->SetGraphVariableBool("bLeviInCatchRange", false);
                            manager->DoKratosAction(ActionType::kWeaponCharge, caster);
#endif
                        } else spdlog::info("levi and mjolnir can't arrive!");
                    } else spdlog::info("levi and mjolnir does not exist!");
                } else if (WeaponIdentify::isLeviathanAxe) {
                    if (!levi->isAxeThrowed && manager->IsCanCharge(caster, RelicType::kLeviathanAxe)) {
                        manager->DoKratosAction(ActionType::kWeaponCharge, caster);
                    }
                } else if (WeaponIdentify::isBladeOfChaos) {
                    BladeOfChaos::GetSingleton()->Update(*g_engineTime);
                    BladeOfChaos::GetSingleton()->BuffScorchingSpeed();
                    manager->DoKratosAction(ActionType::kWeaponCharge, caster);
                } else if (WeaponIdentify::isDraupnirSpear || WeaponIdentify::isTrident) {
                    manager->DoKratosAction(ActionType::kWeaponCharge, caster);
                } else if (WeaponIdentify::isMjolnir) {
                    if (!mjolnir->isMjolnirThrowed && manager->IsCanCharge(caster, RelicType::kMjolnir)) {
                        manager->DoKratosAction(ActionType::kWeaponCharge, caster);
                    }
                }
            } else if (formID == manager->spellID.runic) {
                if (caster->HasSpell(manager->SpellFinisherButton)) {
                    manager->DoKratosAction(ActionType::kRage, caster);
                }
            } else if (formID == manager->spellID.finisher) {
                if (caster->HasSpell(manager->SpellRunicButton)) {
                    manager->DoKratosAction(ActionType::kRage, caster);
                }
            } else if (formID == manager->spellID.leviChargeCoolDown) {
                spdlog::debug("levi charge in cooldown...");
            }
#endif
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

        auto playerCharacter = RE::PlayerCharacter::GetSingleton();
        if (!playerCharacter) return EventChecker::kContinue;

#ifdef KRATOS_COMBAT_3
        auto manager = Assets::GetSingleton();
#else
        auto manager = Kratos::GetSingleton();
#endif
        if (!manager) return EventChecker::kContinue;

        if (auto button = static_cast<RE::ButtonEvent*>(event); button) {
            auto device = event->device.get();
            keyCode = GetOffsettedKeyCode(keyCode, device);
            if (keyCode == Config::AxeCallKey) {
                if (button->IsDown()) {playerCharacter->AddSpell(manager->SpellAxeCallButton); playerCharacter->SetGraphVariableBool("bPressingCallButton", true);}
                else if (button->IsUp()) {playerCharacter->RemoveSpell(manager->SpellAxeCallButton); playerCharacter->SetGraphVariableBool("bPressingCallButton", false);}
            }
#ifdef KRATOS_COMBAT_3
#else
            else if (keyCode == Config::AimKey) {
                if (button->IsDown()) {manager->Aim(true); playerCharacter->AddSpell(manager->SpellAimButton);/* playerCharacter->SetGraphVariableBool("bIsAiming", true);*/}
                else if (button->IsUp()) {manager->Aim(false); playerCharacter->RemoveSpell(manager->SpellAimButton);/* playerCharacter->SetGraphVariableBool("bIsAiming", false);*/}
            }
#endif
            else if (keyCode == Config::RunicKey) {
                if (button->IsDown()) {playerCharacter->AddSpell(manager->SpellRunicButton);}
                else if (button->IsUp()) {playerCharacter->RemoveSpell(manager->SpellRunicButton);}
            }
            else if (keyCode == Config::FinisherKey) {
                if (button->IsDown()) {playerCharacter->AddSpell(manager->SpellFinisherButton);}
                else if (button->IsUp()) {playerCharacter->RemoveSpell(manager->SpellFinisherButton);}
            }
            else if (keyCode == Config::MediumDistanceKey) {
                if (button->IsDown()) {playerCharacter->AddSpell(manager->SpellMidDistButton);}
                else if (button->IsUp()) {playerCharacter->RemoveSpell(manager->SpellMidDistButton);}
            }
            else if (keyCode == Config::LongDistanceKey) {
                if (button->IsDown()) {playerCharacter->AddSpell(manager->SpellLongDistButton);}
                else if (button->IsUp()) {playerCharacter->RemoveSpell(manager->SpellLongDistButton);}
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
#ifdef KRATOS_COMBAT_3
        RelicManager::GetSingleton()->OnMenuOpenCloseEvent(a_event->opening);
#else
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
#endif
    } return EventChecker::kContinue;
}
