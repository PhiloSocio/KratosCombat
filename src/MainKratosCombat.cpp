#include "util.h"
#include "MainKratosCombat.h"
//#include "API/TrueDirectionalMovementAPI.h"

using namespace Util;
using tState = LeviathanAxe::ThrowState;
//static std::jthread axeEquipThread;
//static std::jthread axeChargeThread;
static void DelayedCast(RE::Actor* a_caster, RE::SpellItem* a_spell, RE::NiNode* a_target, float a_delaySec)
{
    if (!a_spell) return;
    if (!a_caster) return;
    int delay = static_cast<int>(a_delaySec * 1000.f);
    spdlog::debug("draupnir's call start...");
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    if (!a_spell) return;
    if (!a_caster) return;
    spdlog::debug("draupnir's call triggered!");
//  a_caster->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(a_spell, false, nullptr, 1.f, false, 10.f, a_caster);
    
    if (!a_target) return;
    RE::ProjectileHandle pHandle;
    RE::NiPoint3 startPoint = a_target->world.translate + RE::NiPoint3(0.f, 0.f, 128.f);
    RE::Projectile::ProjectileRot pRot = {a_caster->GetAimAngle(), a_caster->GetAimHeading()};
    RE::Projectile::LaunchSpell(&pHandle, a_caster, a_spell, startPoint, pRot);
    spdlog::debug("draupnir's call end!");
}

//WEAPON IDENTIFICATION
void WeaponIdentify::WeaponIdentifier(RE::PlayerCharacter* a_player, RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectARMO* a_shield)
{
    auto RelicName = "not a Relic";
//  auto address = reinterpret_cast<std::uintptr_t>(a_RHandWeapon);
    if (a_RHandWeapon) {
        if (Config::LeviathanAxeKWD && a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {
            isLeviathanAxe = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Leviathan Axe";
            LeviathanAxe = a_RHandWeapon;

            auto Levi = LeviathanAxe::GetSingleton();
            Levi->data.weap     = LeviathanAxe;
            Levi->data.ench     = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_player, false);
            Levi->data.poison   = ObjectUtil::Poison::GetEquippedObjPoison(a_player, false);
            Levi->data.damage   = static_cast<float>(LeviathanAxe->attackDamage);
        //  Levi->LeviProjBaseL->model = LeviathanAxe->model;
        //  Levi->LeviProjBaseH->model = LeviathanAxe->model;
        //  Levi->LeviProjBaseA->model = LeviathanAxe->model;
            if (WeaponIdentify::LeviathanAxe->HasWorldModel()) {spdlog::debug("Levi is throwable fID {}, rID {}, lID {}", LeviathanAxe->formID, LeviathanAxe->GetRawFormID(), LeviathanAxe->GetLocalFormID()); Levi->SetThrowState(tState::kThrowable);}
            else spdlog::debug("Levi is not equipped for real");

            if (Levi->SpellCatchLevi && a_player->AsMagicTarget()->HasMagicEffect(Levi->EffCatchLevi)) {a_player->RemoveSpell(Levi->SpellCatchLevi);}

            if (Levi->LeviathanAxeProjectileA 
            && (Levi->GetThrowState() == tState::kArriving
             || Levi->GetThrowState() == tState::kArrived)) 
                Levi->Catch(Levi->LeviathanAxeProjectileA, a_player, true);
        }
        else if (Config::BladeOfChaosKWD && a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {
            isBladeOfChaos = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Blade of Chaos";
            BladeOfChaos = a_RHandWeapon;
        }
        else if (Config::DraupnirSpearKWD && a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {
            isDraupnirSpear = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Draupnir Spear";
            DraupnirSpear = a_RHandWeapon;
        }
        else if (Config::BladeOfOlympusKWD && a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {
            isBladeOfOlympus = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Blade of Olympus";
            BladeOfOlympus = a_RHandWeapon;
        }
        spdlog::info("{} is {}", a_RHandWeapon->GetName(), RelicName);
    } if (a_shield) {
        RelicName = "not a Relic";
        if (Config::GuardianShieldKWD && a_shield->HasKeyword(Config::GuardianShieldKWD)) {
            isKratos = true;
            RelicName = "the Guardian Shield";
            GuardianShield = a_shield;
        }
        spdlog::info("{} is {}", a_shield->GetName(), RelicName);
    } if (isKratos) return;
}
void WeaponIdentify::WeaponCheck()
{
    isLeviathanAxe  = false;
    isBladeOfChaos  = false;
    isDraupnirSpear = false;
    isRelic = false;
    isKratos = false;

    auto AnArchos = PlayerCharacter::GetSingleton();
    if      (!AnArchos) return;
    if      (AnArchos->GetNodeByName("NPC R Finger20 [RF20]"))  {RHandBone = AnArchos->GetNodeByName("NPC R Finger20 [RF20]");}
    else if (AnArchos->GetNodeByName("NPC R MagicNode [RMag]")) {RHandBone = AnArchos->GetNodeByName("NPC R MagicNode [RMag]");}
    else if (AnArchos->GetNodeByName("NPC R Hand [RHnd]"))      {RHandBone = AnArchos->GetNodeByName("NPC R Hand [RHnd]");}

    if      (AnArchos->GetNodeByName("NPC L Finger20 [LF20]"))  {LHandBone = AnArchos->GetNodeByName("NPC L Finger20 [LF20]");}
    else if (AnArchos->GetNodeByName("NPC L MagicNode [LMag]")) {LHandBone = AnArchos->GetNodeByName("NPC L MagicNode [LMag]");}
    else if (AnArchos->GetNodeByName("NPC L Hand [LHnd]"))      {LHandBone = AnArchos->GetNodeByName("NPC L Hand [LHnd]");}

    if  (AnArchos->GetNodeByName("Weapon"))                     {WeaponBone = AnArchos->GetNodeByName("Weapon");}
    if  (AnArchos->GetNodeByName("Shield"))                     {ShieldBone = AnArchos->GetNodeByName("Shield");}
    //  spdlog::debug("Right hand bone is {}", RHandBone->name);

    auto pcSkillArchery = AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kArchery);
    auto pcSkill1Handed = AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kOneHanded);
    auto pcDamageMult   = AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAttackDamageMult);
    DamageMult  = 1.f + (pcSkill1Handed / 120) + (pcSkillArchery / 80);
    DamageMult  *= pcDamageMult;

    RE::TESObjectWEAP* RHandWeapon = nullptr;
    RE::TESObjectARMO* shield = nullptr;
    if (auto rObj = AnArchos->GetEquippedObject(false)) {
        if (rObj->IsWeapon()) {
            RHandWeapon = rObj->As<TESObjectWEAP>();
        }
    } if (auto lObj = AnArchos->GetEquippedObject(true)) {
        if (lObj->IsArmor()) {
            shield = lObj->As<TESObjectARMO>();
        }
    } if (RHandWeapon || shield) return WeaponIdentifier(AnArchos, RHandWeapon, shield);
}
bool WeaponIdentify::IsRelic(RE::BGSProjectile *a_baseProj, bool a_onlyLevi, bool a_onlyDraupnir)
{
    if (!a_baseProj) return false;
    auto Levi = LeviathanAxe::GetSingleton();
    auto leviA = Levi->LeviProjBaseA;
    auto leviL = Levi->LeviProjBaseL;
    auto leviH = Levi->LeviProjBaseH;
    auto draup = Draupnir::DraupnirSpearProjBaseL;
    if (a_onlyLevi) return (a_baseProj == leviA || a_baseProj == leviL || a_baseProj == leviH);
    else if (a_onlyDraupnir) return (a_baseProj == draup);

    return (a_baseProj == leviA || a_baseProj == leviL || a_baseProj == leviH || a_baseProj == draup);
}
//KRATOS
bool Kratos::Initialize()
{
    bool result = false;
    EffectAxeThrownState= SpellAxeThrownState ? SpellAxeThrownState->effects[0]->baseEffect : nullptr;
    EffectSpartanRage   = SpellSpartanRage ? SpellSpartanRage->effects[0]->baseEffect : nullptr;
    EffectRageCoolDown  = SpellRageCoolDown ? SpellRageCoolDown->effects[0]->baseEffect : nullptr;
    EffectLeviChargeCD  = SpellLeviChargeCD ? SpellLeviChargeCD->effects[0]->baseEffect : nullptr;
    EffectAimButton     = SpellAimButton ? SpellAimButton->effects[0]->baseEffect : nullptr;
    EffectAxeCallButton = SpellAxeCallButton ? SpellAxeCallButton->effects[0]->baseEffect : nullptr;
    EffRunicButton      = SpellRunicButton   ? SpellRunicButton ->effects[0]->baseEffect : nullptr;
    EffFinisherButton   = SpellFinisherButton ? SpellFinisherButton->effects[0]->baseEffect : nullptr;
    EffMidDistButton    = SpellMidDistButton ? SpellMidDistButton->effects[0]->baseEffect : nullptr;
    EffLongDistButton   = SpellLongDistButton ? SpellLongDistButton->effects[0]->baseEffect : nullptr;
    result = EffectAxeThrownState && EffectSpartanRage && EffectRageCoolDown && EffectLeviChargeCD && EffectAimButton && EffectAxeCallButton
    && EffRunicButton && EffFinisherButton;
    if (result) {
        spellID.aim = EffectAimButton->GetFormID();
        spellID.call = EffectAxeCallButton->GetFormID();
        spellID.runic = EffRunicButton->GetFormID();
        spellID.finisher = EffFinisherButton->GetFormID();
        spellID.midDist = EffMidDistButton->GetFormID();
        spellID.longDist = EffLongDistButton->GetFormID();
        spellID.leviChargeCoolDown = EffectLeviChargeCD->GetFormID();
        spellID.AxeThrownState = EffectAxeThrownState->GetFormID();
        spellID.rageCoolDown = EffectRageCoolDown->GetFormID();
        spellID.spartanRage = EffectSpartanRage->GetFormID();
    } return result;
}
bool Kratos::IsEquipped(const Kratos::Relic a_relic)
{
    switch (a_relic) {
    case Relic::kNone:
        return WeaponIdentify::isRelic;
    case Relic::kLeviathanAxe:
        return WeaponIdentify::isLeviathanAxe;
    case Relic::kBladeOfChaos:
        return WeaponIdentify::isBladeOfChaos;
    case Relic::kDraupnirSpear:
        return WeaponIdentify::isDraupnirSpear;
    case Relic::kBladeOfOlympus:
        break;
    } return false;
}
bool Kratos::IsCanCallAxe(RE::Actor* a_actor)   { if (a_actor && SpellAxeThrownState) return a_actor->HasSpell(SpellAxeThrownState); return false;}
bool Kratos::IsCanRage(RE::Actor* a_actor)
{
    if (a_actor && a_actor->AsMagicTarget() && EffectRageCoolDown) {
        return !(a_actor->AsMagicTarget()->HasMagicEffect(EffectRageCoolDown));
    } return false;
}
bool Kratos::IsCanCharge(RE::Actor* a_actor, Kratos::Relic a_relic)
{
    bool isCan = false;
    auto magicTarget = a_actor->GetMagicTarget();
    auto cdLeviEffect = SpellLeviChargeCD ? SpellLeviChargeCD->effects[0] : nullptr;
    auto cdLeviBaseEffect = cdLeviEffect ? cdLeviEffect->baseEffect : nullptr;
    if (magicTarget) {
        switch  (a_relic) {
        case Relic::kLeviathanAxe:
            if (a_actor && cdLeviBaseEffect) isCan = !magicTarget->HasMagicEffect(cdLeviBaseEffect);
            break;
        case Relic::kBladeOfChaos:
            break;
        case Relic::kDraupnirSpear:
            break;
        default:
            break;
        }
    } return isCan;
}
bool Kratos::IsShieldOpened() {return _isShieldOpened;}
void Kratos::SetIsCanCallAxe(RE::Actor* a_actor, const bool a_isCan)
{
    if (a_isCan && !IsCanCallAxe(a_actor)) a_actor->AddSpell(SpellAxeThrownState);
    else if (!a_isCan && IsCanCallAxe(a_actor)) a_actor->RemoveSpell(SpellAxeThrownState);
}
void Kratos::SetIsCanRage(RE::Actor* a_actor, const bool a_isCan)
{
    if (a_isCan && !IsCanRage(a_actor)) {
        auto casterHandle = a_actor->GetHandle();
        auto magicTarget = a_actor->GetMagicTarget();
        magicTarget->DispelEffect(SpellRageCoolDown, casterHandle);
    }
    else if (!a_isCan && IsCanRage(a_actor)) 
        a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(SpellRageCoolDown, false, nullptr, 1.f, false, 1.f, a_actor);
}
void Kratos::SetIsCanCharge(RE::Actor* a_actor, const bool a_isCan, Kratos::Relic a_relic)
{
    auto magicTarget = a_actor->GetMagicTarget();
    if (a_isCan && !IsCanCharge(a_actor, a_relic) && magicTarget) {
        auto casterHandle = a_actor->GetHandle();

        switch (a_relic) {
        case Relic::kLeviathanAxe:
            magicTarget->DispelEffect(SpellLeviChargeCD, casterHandle);
            break;
        case Relic::kBladeOfChaos:
            break;
        case Relic::kDraupnirSpear:
            break;
        default:
            break;
        }
    }
    else if (!a_isCan && IsCanCharge(a_actor, a_relic) && magicTarget) {
        switch (a_relic) {
        case Relic::kLeviathanAxe:
            a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(SpellLeviChargeCD, false, nullptr, 1.f, false, 1.f, a_actor);
            break;
        case Relic::kBladeOfChaos:
            break;
        case Relic::kDraupnirSpear:
            break;
        default:
            break;
        }
    }
}
void Kratos::SetCoolDownOf(const Kratos::Skill a_skill, const uint8_t a_coolDownSec)
{
    switch  (a_skill) {
    case Skill::kRage:
        ObjectUtil::Spell::SetDuration(SpellRageCoolDown, a_coolDownSec);
        break;
    case Skill::kWeaponCharge:
        ObjectUtil::Spell::SetDuration(SpellLeviChargeCD, a_coolDownSec);
        break;
    case Skill::kRunicAttack:
    //  ObjectUtil::Spell::SetDuration(SpellRunicCoolDown, a_coolDownSec);
        break;
    }
}
void Kratos::OpenShield(RE::Actor* a_actor)
{
    if (a_actor) {
        auto shieldModel = a_actor->GetNodeByName("NiNode002");
        if (auto shield = WeaponIdentify::GuardianShield) {
            if (shieldModel) {
                if (auto controllers = shieldModel->GetControllers()) {
                    if (auto manager = controllers->AsNiControllerManager()) {
                        auto startSequence = manager->GetSequenceByName("Begin");
                        auto endSequence = manager->GetSequenceByName("Loop");
                        if (startSequence && endSequence) {
                            PlayerCharacter::GetSingleton()->PlayAnimation(manager, endSequence, startSequence);
                            _isShieldOpened = true;
                        } else spdlog::debug("can't find the sequences");
                    }
                }
            } else spdlog::debug("can't find the guardian shield model");
        } else spdlog::debug("can't find the guardian shield");
    }
}
void Kratos::CloseShield(RE::Actor* a_actor)
{
    if (a_actor) {
        if (auto shield = WeaponIdentify::GuardianShield) {
            if (auto shieldModel = shield->As<RE::NiAVObject>()) {
                if (auto controllers = shieldModel->GetControllers()) {
                    if (auto manager = controllers->AsNiControllerManager()) {
                        auto startSequence = manager->GetSequenceByName("End");
                        auto endSequence = manager->GetSequenceByName("Idle");
                        if (startSequence && endSequence) {
                            PlayerCharacter::GetSingleton()->PlayAnimation(manager, endSequence, startSequence);
                            _isShieldOpened = false;
                        } else spdlog::debug("can't find the sequences");
                    }
                }
            } else spdlog::debug("can't find the guardian shield model");
        } else spdlog::debug("can't find the guardian shield");
    }
}
void Kratos::StartRage(RE::Actor* a_actor, const Kratos::Rage a_rage)
{
    if (a_actor) {
        switch (a_rage)
        {
        case Kratos::Rage::kSpartan:
            a_actor->NotifyAnimationGraph("IdleCombatStart");
            a_actor->AddSpell(SpellSpartanRage);
            a_actor->AddSpell(SpellRageCoolDown);
            if (VFXeffect.spartanRage) a_actor->ApplyArtObject(VFXeffect.spartanRage, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);
            break;
        case Kratos::Rage::kValor:
            a_actor->NotifyAnimationGraph("IdleCombatStart");
            a_actor->AddSpell(SpellRageCoolDown);
            if (VFXeffect.valorRage) a_actor->ApplyArtObject(VFXeffect.valorRage, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);
            break;
        case Kratos::Rage::kWrath:
            break;
        case Kratos::Rage::kOlympus:
            if (WeaponIdentify::BladeOfOlympus) {
                ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::BladeOfOlympus->formID);
                a_actor->AddSpell(SpellRageCoolDown);
                if (VFXeffect.olympusRage) a_actor->ApplyArtObject(VFXeffect.olympusRage, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);
            }
            break;

        default:
            break;
        }
    }
}
void Kratos::EndRage(RE::Actor* a_actor)
{
    if (a_actor) {
        a_actor->NotifyAnimationGraph("IdleCombatStart");
        a_actor->RemoveSpell(SpellSpartanRage);
    }
}
//LEVIATHAN AXE
LeviathanAxe* LeviathanAxe::GetSingleton()                      {static LeviathanAxe singleton; return &singleton;}
tState LeviathanAxe::GetThrowState()    const   {return throwState;}
void LeviathanAxe::SetThrowState(ThrowState a_state)            {throwState = a_state;}
void LeviathanAxe::SetStartPos(RE::NiPoint3& a_point, RE::PlayerCharacter* a_caller)
{
    auto stuckedLevi =  LastLeviProjectile;
    if (!stuckedLevi) stuckedLevi = (LeviathanAxeProjectileL ? LeviathanAxeProjectileL : LeviathanAxeProjectileH);
    if (stuckedLevi) {
        a_point = stuckedLevi->data.location; 
    } else spdlog::debug("we cant get leviathan's stucked proj!");

    if (data.stuckedBone) {
        a_point = data.stuckedBone->world.translate;
        data.stuckedBone = nullptr;

        if (data.stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
            ObjectUtil::Projectile::DeleteAnExtraArrow(data.stuckedActor, data.model);
#else
            data.stuckedActor->RemoveExtraArrows3D();
#endif
            spdlog::debug("levi stucked actor's extra arrows removed");
            data.stuckedActor = nullptr;
        } else spdlog::debug("levi not stucked anybody");
    } else spdlog::debug("levi not stucked any bone");

    auto pcCell = a_caller->parentCell;
    auto bound = pcCell->GetRuntimeData().cellData.exterior->worldX;
    auto pcPos = a_caller->GetPosition();
    float dist = pcPos.GetDistance(a_point);
    if (dist > 36000.f) {   // ~42000 is limit
        spdlog::info("levi too far from you! ({} m)", (int)dist / 100);
        auto dir = a_point - pcPos;
        dir.Unitize();
        a_point = pcPos + dir * 36000.f;
    }
}
void LeviathanAxe::Throw(bool isVertical)
{
    const auto AnArchos = PlayerCharacter::GetSingleton();

    const auto leviThrowSpell = (isVertical ? SpellLeviProjH : SpellLeviProjL);
//  auto leviBaseProj = (isVertical ? LeviProjBaseH : LeviProjBaseL);
    if (WeaponIdentify::LeviathanAxe && leviThrowSpell && AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe) 
    {   //  calculate damage
        const auto leviDamage = static_cast<float>(WeaponIdentify::LeviathanAxe->attackDamage);
        float mag = leviDamage * WeaponIdentify::DamageMult;
        if (isVertical) {mag *= 1.5f; data.yAngle = 1.57f;}
        else data.yAngle = 0.35f;
        const auto leviProjEff = leviThrowSpell->effects[0];
        auto& leviProjEffSetting = leviProjEff->effectItem;
        leviProjEffSetting.magnitude = mag;

    //  if (const auto leviProjBaseEff = leviProjEff->baseEffect) {
    //      leviProjBaseEff->data.projectileBase->SetModel(WeaponIdentify::LeviathanAxe->GetModel());
    //  //  leviProjBaseEff->data.projectileBase->data.defaultWeaponSource = WeaponIdentify::LeviathanAxe;
    //  //  leviProjBaseEff->data.associatedForm = WeaponIdentify::LeviathanAxe;
    //  } else spdlog::warn("not found throwing effect!");
//
//      auto extraData = AnArchos->extraList;
//      extraData.SetInventoryChanges(RE::InventoryChanges)
    //  RE::ActorEquipManager::GetSingleton()->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, false, false, false, true, nullptr);
    //  AnArchos->RemoveWeapon(RE::BIPED_OBJECT::kOneHandAxe);
    //  AnArchos->NotifyAnimationGraph("UnequipNoAnim");
//
    //  AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(leviThrowSpell, false, nullptr, 1.f, false, mag, AnArchos);
    //  auto origin = AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->GetMagicNode()->world.translate;

        auto kratos = Kratos::GetSingleton();
        WeaponIdentify::skipEquipAnim = true;
        //  set the launch data
        auto origin = WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = {AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
        RE::Projectile::LaunchData lData(AnArchos, origin, pRot, leviThrowSpell);

        lData.weaponSource = data.weap;
        lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(AnArchos, false);
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);
        //  throw the projectile
        RE::Projectile::Launch(&pHandle, lData);

        kratos->SetIsCanCallAxe(AnArchos);
        isAxeCalled = false;
        isAxeThrowed = true;
        SetThrowState(ThrowState::kThrown);
            spdlog::info("Leviathan Axe throwed, raw damage is: {}", mag);
        //  reset the last throw's traces
        if (data.stuckedBone)   data.stuckedBone    = nullptr;
        if (data.stuckedActor)  data.stuckedActor   = nullptr;
        if (data.lastHitActor)  data.lastHitActor   = nullptr;
        if (AnArchos->HasSpell(SpellCatchLevi)) AnArchos->RemoveSpell(SpellCatchLevi);
        //  go to barehanded state
    //  const auto eqManager = RE::ActorEquipManager::GetSingleton();
    //  eqManager->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe, nullptr, 1U, nullptr, true, false, true, true);
    } else spdlog::info("Leviathan Axe is not equipped for throwing!");
}
void LeviathanAxe::Call()
{
    if (data.weap) {
        spdlog::debug("Levi is calling...");
        auto stuckedLevi =  LastLeviProjectile ? LastLeviProjectile : nullptr;
        if (!stuckedLevi)   stuckedLevi = (LeviathanAxeProjectileL ? LeviathanAxeProjectileL : (LeviathanAxeProjectileH ? LeviathanAxeProjectileH : nullptr));
        const auto AnArchos = PlayerCharacter::GetSingleton();

        if (SpellLeviProjA) {
            isAxeCalled = true;
            isAxeThrowed = false;

            const auto leviDamage = static_cast<float>(data.weap->attackDamage);
            float mag = leviDamage * WeaponIdentify::DamageMult;
            const auto leviProjEff = SpellLeviProjA->effects[0];
            auto& leviProjEffSetting = leviProjEff->effectItem;
            leviProjEffSetting.magnitude = mag * 0.4f;

            if (stuckedLevi) data.position = stuckedLevi->data.location;
            RE::NiPoint3 startPoint = data.position;
            SetStartPos(startPoint, AnArchos);
            RE::ProjectileHandle pHandle;
            RE::Projectile::ProjectileRot pRot = {-AnArchos->GetAimAngle(), -AnArchos->GetAimHeading()};
            RE::Projectile::LaunchData lData(AnArchos, startPoint, pRot, SpellLeviProjA);
        //  const float dist = WeaponIdentify::RHandBone->world.translate.GetDistance(startPoint);
        //  spdlog::debug("levi arriving from {}cm far away!", dist);

            lData.noDamageOutsideCombat = true; //  can be an option
            lData.weaponSource = data.weap;
            lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(AnArchos, false);
        //  if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
                lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

        //  precalculation of arriving speed
        //  arrivalSpeed = dist / Config::ArrivalTime;
        //  if      (arrivalSpeed < Config::MinArrivalSpeed)    arrivalSpeed = Config::MinArrivalSpeed;
        //  else if (arrivalSpeed > Config::MaxArrivalSpeed)    arrivalSpeed = Config::MaxArrivalSpeed;
        //  LeviProjBaseA->data.speed = arrivalSpeed;

            RE::Projectile::Launch(&pHandle, lData);
        //  RE::Projectile::LaunchSpell(&pHandle, AnArchos, SpellLeviProjA, startPoint, pRot);
            SetThrowState(ThrowState::kArriving);

/*  */  if (stuckedLevi) {
            //  auto& fFlags = stuckedLevi->formFlags;
                auto& runtimeData = stuckedLevi->GetProjectileRuntimeData();
                auto& pFlags = runtimeData.flags;
                if (!(pFlags & (1 << 25))) {
                    pFlags |= (1 << 25);
                //      spdlog::debug("levi destroyed before call");
                } else  spdlog::debug("levi is already destroyed");

            //  if ((fFlags & rFlag::kInitialized)) {
        //      if (!(fFlags & rFlag::kDisabled)) {
        //          if (!(fFlags & rFlag::kStillLoading)) {
        //              auto& runtimeData = stuckedLevi->GetProjectileRuntimeData();
        //              auto& pFlags = runtimeData.flags;
        //              if (!(pFlags & (1 << 25))) pFlags |= ~(1 << 25);
        //              spdlog::debug("levi destroyed before call");
        //              //auto expl = runtimeData.explosion;
        //              //if (expl) {
        //              //  if (pFlags & (1 << 15)) {   //  kDestroyAfterHit flag required, otherwise the explosion will cast every moment
        //              //      if ((pFlags & (1 << 31))) {
        //              //          if (pFlags & (1 << 13)) pFlags &= ~(1 << 13);
        //              //          if (pFlags & (1 << 25)) pFlags &= ~(1 << 25);
        //              //          if (runtimeData.explosionTimer > 0.f) runtimeData.explosionTimer = 0.02f;
        //              //              spdlog::debug("Levi is detached!!");
        //              //      } else {spdlog::debug("Levi is not moved (runtimeFlag)");}
        //              //  } else {spdlog::debug("Levi is not destructable (runtimeFlag)");}
        //              //} else {spdlog::info("Levi not has explosion");}
        //          } else {spdlog::debug("Levi is kStillLoading (formFlag)");}
        //      } else {spdlog::debug("Levi is kDisabled (formFlag)");}
        //  } else {spdlog::debug("Levi is not inited (formFlag)");}
            } else {spdlog::debug("Stucked Levi is nullptr!");}
        } else {spdlog::warn("WEIRD SpellLeviProjA is nullptr!");}
    } else {spdlog::warn("WEIRD you don't have the axe for calling!!");}
    spdlog::info("Levi is arriving...");
}
void LeviathanAxe::Catch(RE::Projectile* a_levi, RE::PlayerCharacter* a_player, bool a_justDestroy)
{
    if (a_player && !a_justDestroy) {
        if (EffCatchLevi && SpellCatchLevi && !a_player->AsMagicTarget()->HasMagicEffect(EffCatchLevi)) {
            a_player->AddSpell(SpellCatchLevi);
        }

        if (throwState == ThrowState::kCanArrive) SetThrowState(ThrowState::kArrived);

        auto kratos = Kratos::GetSingleton();

        if (auto handEffect = kratos->VFXeffect.handFrost) 
            a_player->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);

        if (auto catchSound = kratos->soundEffect.catchLevi) 
            ObjectUtil::Sound::PlaySound(catchSound, WeaponIdentify::RHandBone, 2.f);

        if (WeaponIdentify::LeviathanAxe) {
            auto eqManager = RE::ActorEquipManager::GetSingleton();
        //  auto invChanges = a_player->GetInventoryChanges();
        //  auto entries = invChanges->entryList;
        //  RE::ExtraDataList* xList = nullptr;
        //  for (auto entry : *entries) {
        //      if (entry->object == WeaponIdentify::LeviathanAxe->As<RE::TESBoundObject>()) {
        //          xList = entry->extraLists->front();
        //          break;
        //      }
        //  }
            eqManager->EquipObject(a_player, WeaponIdentify::LeviathanAxe, nullptr, 1U, nullptr, true, false, false, true);
        } else spdlog::warn("you not have the leviathan axe");

        if (data.stuckedBone)   data.stuckedBone    = nullptr;
        if (data.stuckedActor)  data.stuckedActor   = nullptr;
        if (data.lastHitActor)  data.lastHitActor   = nullptr;

        std::jthread delayedCast([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            isAxeCalled = false;
            auto kratos = Kratos::GetSingleton();
            kratos->SetIsCanCallAxe(a_player, false);
        //  spdlog::debug("Levi proj catched");
        });
        delayedCast.detach();
    }
    if (a_levi) {
        auto& runtimeData = a_levi->GetProjectileRuntimeData();
        runtimeData.flags |= (1 << 25);                             //  set as destroyed, RE::Projectile::Flags::kDestroyed
    }
}
void LeviathanAxe::Charge(const uint8_t a_chargeHitCount, const float a_magnitude, const uint8_t a_coolDown)
{
    auto kratos     = Kratos::GetSingleton();
    auto AnArchos   = RE::PlayerCharacter::GetSingleton();
    auto leviWeap   = data.weap;
    auto ench       = leviWeap ? ObjectUtil::Enchantment::GetInventoryItemEnchantment(AnArchos, leviWeap->formID) : nullptr;
    auto enchEffect = ench ? ench->effects[0] : nullptr;
    auto enchBase   = enchEffect ? enchEffect->baseEffect : nullptr;
    const auto leviDam  = data.damage;

    if (leviWeap && chargeHitCount <= 0) {
        if (enchBase) {
            spdlog::debug("levi's enchantment is: {}", ench->GetName());
            if (enchBase->HasArchetype(RE::EffectSetting::Archetype::kDualValueModifier)
             || enchBase->HasArchetype(RE::EffectSetting::Archetype::kValueModifier)) {
                ResetCharge(data.enchMag, data.defaultEnchMag, false, true);
                auto& mag = enchEffect->effectItem.magnitude;
                data.enchMag = &mag;
                data.defaultEnchMag = mag;
                mag *= a_magnitude;
                chargeHitCount = a_chargeHitCount;
                ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 300.f);
                kratos->chargeData.lastBuffedLevi = leviWeap->formID;
                kratos->SetCoolDownOf(Kratos::Skill::kWeaponCharge, a_coolDown);
                kratos->SetIsCanCharge(AnArchos, false);
                spdlog::debug("magnitude buffing from {} to: {}", mag / a_magnitude, mag);
            } else spdlog::debug("levi's enchantment is not expected archetype.");
#ifdef EXPERIMENTAL_CHARGE_LEVI
        } else {
            spdlog::debug("levi not has any enchantment, levi is enchanting...");
            if (EnchCharge) {
                ench = EnchCharge;
                enchEffect  = ench ? ench->effects[0] : nullptr;
                enchBase    = enchEffect ? enchEffect->baseEffect : nullptr;
                if (enchBase) {
                    ResetCharge(data.enchMag, data.defaultEnchMag, false, true);
                    ObjectUtil::Enchantment::EnchantEquippedWeapon(AnArchos, ench, 300.f, false, false);
                    ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 300.f);
                    kratos->chargeData.lastChargedLevi = leviWeap->formID;

                //  AnArchos->GetActorRuntimeData().emotionType = RE::EmotionType::kAnger;
                //  AnArchos->GetActorRuntimeData().emotionValue = 100;

                    auto& enchCost = ench->data.costOverride;
                    auto& enchAmount = ench->data.chargeOverride;
                    auto& mag = enchEffect->effectItem.magnitude;
                    enchAmount = 500.f;
                    mag = a_magnitude * leviDam / 2;
                    enchCost = mag;
                    data.enchMag = nullptr;

                    chargeHitCount = a_chargeHitCount;
                    kratos->SetCoolDownOf(Kratos::Skill::kWeaponCharge, a_coolDown);
                    kratos->SetIsCanCharge(AnArchos, false);
                    spdlog::info("levi charge frost damage buff is: {}", mag);

                    auto weapBone = WeaponIdentify::WeaponBone;
                    if (auto handEffect = kratos->VFXeffect.handFrostBright) AnArchos->ApplyArtObject(handEffect, a_chargeHitCount * 2, nullptr, false, false, weapBone);
                    else spdlog::warn("can't found hand effect for levi charge!");

                    if (auto soundEffect = kratos->soundEffect.chargeLeviEnd) ObjectUtil::Sound::PlaySound(soundEffect, weapBone, 5.f);
                } else spdlog::error("can't find frost enchantment's base!!");
            } else spdlog::error("can't find frost enchantment!!");
#endif
        }
    } else spdlog::error("can't find levi for charging!!");
}
void LeviathanAxe::ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck, const bool a_justReset)
{
    auto kratos = Kratos::GetSingleton();
    if (!kratos) return;
    if (a_justReset) {
        if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), kratos->chargeData.lastChargedLevi) == EnchCharge)
            ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), kratos->chargeData.lastChargedLevi);
    } else if (kratos->IsEquipped(Kratos::Relic::kLeviathanAxe)) {
        if (chargeHitCount <= 0) {
            if (a_magnitude) *a_magnitude = a_defMagnitude;
            else {
                if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), kratos->chargeData.lastChargedLevi) == EnchCharge)
                    ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), kratos->chargeData.lastChargedLevi);
            }
            kratos->SetIsCanCharge(RE::PlayerCharacter::GetSingleton());
        } else if (!a_justCheck) {chargeHitCount -= 1;}
    }
}
void LeviathanAxe::SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical)
{
    if  (a_vertical) {
        a_matrix.entry[0][0] = 0.02f;   //  const
        a_matrix.entry[0][1] = -0.54f;  //  same
        a_matrix.entry[0][2] = 0.84f;   //  
        a_matrix.entry[1][0] = 0.01f;   //  const
        a_matrix.entry[1][1] = 0.84f;   //  
        a_matrix.entry[1][2] = 0.54f;   //  
        a_matrix.entry[2][0] = -1.f;    //  const
        a_matrix.entry[2][1] = 0.0f;    //  const same
        a_matrix.entry[2][2] = 0.02f;   //  const
    } else {
        a_matrix.entry[0][0] = 0.79f;   //  
        a_matrix.entry[0][1] = -0.54f;  //  same
        a_matrix.entry[0][2] = 0.29f;   //  
        a_matrix.entry[1][0] = 0.51f;   //  
        a_matrix.entry[1][1] = 0.84f;   //  same
        a_matrix.entry[1][2] = 0.19f;   //  
        a_matrix.entry[2][0] = -0.34f;  //  const
        a_matrix.entry[2][1] = 0.0f;    //  const same
        a_matrix.entry[2][2] = 0.93f;   //  const
    }

//  vertical axe turn
//  [0.018283, 0.011742, -0.999764]
//  [-0.540305, 0.841469, 0.000002]
//  [0.841270, 0.540178, 0.021729]
//
//  lateral axe turn
//  [0.790722, 0.507721, -0.342020]
//  [-0.540305, 0.841469, 0.000002]
//  [0.287801, 0.184794, 0.939693]
}
void LeviathanAxe::SetHitRotation(RE::NiPoint3& a_angles, const RE::NiPoint3& a_direction, const bool a_vertical)
{
//  a_angles.x = asin(a_direction.z);
    a_angles.z = atan2(a_direction.x, a_direction.y);
    if  (a_vertical) {
        a_angles.x += Config::MinAxeStuckAngle; //  const
    //  a_angles.y = ;          //= -88.6f;     //  const
    //  a_angles.z = ;          //+= -32.7f;    //  
    } else {
    //  a_angles.x = 0.f;                       //  const
    //  a_angles.y = ;          //= -20.f;      //  const
        a_angles.z += Config::MinAxeStuckAngle; //+= -32.7f;    //  
    }
}
void LeviathanAxe::TweakHitPosition(RE::NiPoint3& a_position, const RE::NiPoint3& a_direction, const float a_offset, const bool a_vertical)
{
    auto rightDir   = a_direction.UnitCross(upVec);
    auto leftDir    = a_direction.UnitCross(downVec);
    auto upDir      = a_direction.UnitCross(rightDir);
    auto downDir    = a_direction.UnitCross(leftDir);

    a_position -= (a_direction * a_offset);
    if (a_vertical) {
        a_position += (downDir * a_offset);
    } else {
        a_position += (leftDir * a_offset);
    }
}
//DRAUPNIR SPEAR
void Draupnir::Throw()
{
    const auto AnArchos = PlayerCharacter::GetSingleton();
//  auto& runtimeData = AnArchos->GetActorRuntimeData();
    if (WeaponIdentify::DraupnirSpear && SpellDraupnirProjL && AnArchos->GetEquippedObject(false) == WeaponIdentify::DraupnirSpear)
    {
        const auto draupnirDamage = static_cast<float>(WeaponIdentify::DraupnirSpear->attackDamage);
        float mag = draupnirDamage * WeaponIdentify::DamageMult;
        const auto effDraupnir = SpellDraupnirProjL->effects[0];
        auto& leviProjEffSetting = effDraupnir->effectItem;
        leviProjEffSetting.magnitude = mag;

        auto origin = WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = {AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
        RE::Projectile::LaunchData lData(AnArchos, origin, pRot, SpellDraupnirProjL);
        lData.weaponSource = WeaponIdentify::DraupnirSpear;
        if (lData.weaponSource->formEnchanting) lData.enchantItem = lData.weaponSource->formEnchanting;
        else spdlog::debug("draupnir not has a valid enchantment");

        RE::Projectile::Launch(&pHandle, lData);
    //  auto effBaseDraupnir = effDraupnir->baseEffect;
    //  if (effBaseDraupnir) {
    //      effBaseDraupnir->data.projectileBase->data.defaultWeaponSource = WeaponIdentify::LeviathanAxe;
    //      effBaseDraupnir->data.associatedForm = WeaponIdentify::LeviathanAxe;
    //  } else spdlog::warn("not found throwing effect!");
    //  DraupnirSpearProjBaseL->data.defaultWeaponSource = WeaponIdentify::DraupnirSpear;
    //  AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(SpellDraupnirProjL, false, nullptr, 1.f, false, mag, AnArchos);
    } else  spdlog::info("Draupnir Spear is not equipped for throwing");
}
void Draupnir::Call(const float a_damage, const float a_force)  //detonate throwed draupnir spears like ragnarok
{
    int i = 0;  //  spear counter
    int j = 1;  //  explodable spear counter
    for (auto proj : DraupnirSpearProjectiles) {
        i++;
        if (proj) {
            const auto& fFlags = proj->formFlags;
                spdlog::debug("{}. Draupnir flags: {:032b}", i, fFlags);
                /*  [crashed flags]
                00111001010010000101010001101000
                00111100110011110000010000000000
                10111111001001100001100000000000    // IMPORTANT CRASH
                00000000000000000000000000011110    // stucked to characters.
                00000000000000000000000000000 1 00  // stucked to characters. explosion timer working but not exploding. formID is null. kPlayable
                000000000000000000000010010 1 0000  // kNonOccluder. 4. from right
                000000000000000000000000000111 1 0  // kAltered. 1. from right
                01000011 1 00000000000000000000000  // kDestroyed. 23. from right
                00100011010011010110010111101000    // not has the kmoved flag but is stucked to a non actor object already
                11101011010000001011010110011000    // stucked non living object
                */
                /*  [working flags]
                00000000000000000000000000001000    // kInitialized. (3. from right)
                00000000000000000000000000101000    // kDeleted (5. from right) is not problem
                00000000000000000000001000111000    // it says not has explosion
                */
        //  if (fFlags & rFlag::kDestroyed) {fFlags &= ~rFlag::kDestroyed; spdlog::info("{}. Draupnir's destroyed flag removed", i);}
            if ((fFlags & rFlag::kInitialized)) {
                if (!(fFlags & rFlag::kDisabled)) {
                    if (!(fFlags & rFlag::kStillLoading)) {
                        auto& runtimeData = proj->GetProjectileRuntimeData();
                        auto& pFlags = runtimeData.flags;
                            spdlog::debug("{}. Draupnir runtime flags: {:032b}", i, pFlags);
                            /*  [runtime flags]
                            10000100011100011000000001000000 // IMPORTANT CRASH
                            01111111011111111111111111111111 // crashed with all zero formFlags
                            01000111000100001111101111110001 // not has kmoved flag while stucked to the ground

                            10000100011100011000000101000000 // stucked to objects.
                            kGravityUpdateModel, kInited, kDestroyAfterHit, kAddedToManager, kAddedVisualEffectOnGround, kAutoAim, kProcessedImpacts

                            10000110001101011000000001000000 // stucked to characters.
                            10000110001100011000000001000000 // stucked to characters. explosion time edit working but not exploding.
                            10000110001100011000000001000000 // stucked to characters. explosion time edit working but not exploding. 3d is dissapeared.
                            10100110000100011000000001000000

                            10000110001100011000000011000000 // it says not has explosion

                            10000100011100011000000101000100 // to havok.   kUnk2 = kIsHavok
                            10000110001100011000000001000100 // to havok, caused ctd

                            10000100001000011000000101000000 // still moving.
                            kGravityUpdateModel, kInited, kDestroyAfterHit, kAddedToManager, kAutoAim,

                            10000110001100011000000001000000 // i saw it with kNone form flags.

                            10000100011100011000000001000000 // exploded, explosion timer < 0, formID = 0x0
                            */
#ifdef EXPERIMENTAL
                        if  (!(pFlags & (1 << 8))) {
                            const auto hitBone = DraupnirSpearHitBones[i - 1];
                            const auto target = DraupnirSpearHitActors[i - 1];
                            if (hitBone && target && DraupnirsCallProjBaseL && SpellDraupnirsCallProjL) {
                                const float dist = 6969.f;
                                const float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.1f);
                                const auto delay = randomFloat + static_cast<float>(j) / 4.f;
                                DraupnirsCallProjBaseL->data.speed = dist / delay;

                                const auto AnArchos = RE::PlayerCharacter::GetSingleton();
                                RE::ProjectileHandle pHandle;
                                RE::NiPoint3 startPoint = hitBone->world.translate + RE::NiPoint3(0.f, 0.f, dist);
                                RE::Projectile::ProjectileRot pRot = {0.f, 0.f};    //{AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
                                RE::Projectile::LaunchSpell(&pHandle, AnArchos, SpellDraupnirsCallProjL, startPoint, pRot);

                                proj = nullptr;

                                spdlog::debug("!!experimental debug 1");
                            } else spdlog::debug("!experimental debug 1.1");
                        //  if (!(pFlags & (1 << 2)) && !(pFlags & (1 << 22)) && (pFlags & (1 << 21)) && (pFlags & (1 << 25))) {    //  means hitted to a living target
                        //      spdlog::debug("!!experimental debug 0.1");
                        //      if (!SpellDraupnirsCallProjL) return;
                        //  //  experimental
                        //      const auto AnArchos = RE::PlayerCharacter::GetSingleton();
                        //      for (auto hitBone : DraupnirSpearHitBones) {
                        //          const auto target = DraupnirSpearHitActors[i - 1];
                        //          if (hitBone && target && DraupnirsCallProjBaseL) {
                        //              const float dist = 6969.f;
                        //              const float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.1f);
                        //              const auto delay = randomFloat + static_cast<float>(j) / 4.f;
                        //              DraupnirsCallProjBaseL->data.speed = dist / delay;
//
                        //              RE::ProjectileHandle pHandle;
                        //              RE::NiPoint3 startPoint = hitBone->world.translate + RE::NiPoint3(0.f, 0.f, dist);
                        //              RE::Projectile::ProjectileRot pRot = {0.f, 0.f};    //{AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
                        //              RE::Projectile::LaunchSpell(&pHandle, AnArchos, SpellDraupnirsCallProjL, startPoint, pRot);
                        //              proj = nullptr;
//
                        //              spdlog::debug("!!experimental debug 1");
//
                        //          //  std::jthread delayedCast([=]() {
                                    //      DelayedCast(AnArchos, SpellDraupnirsCallProjL, hitBone, delay);
                                    //      target->RemoveExtraArrows3D();
                                    //      DraupnirSpearHitBones[i - 1] = nullptr;
                                    //      DraupnirSpearHitActors[i - 1] = nullptr;
                                    //  });
                                    //  delayedCast.detach();
                        //          }
                        //      }
                        //      j++;
                        //  }
                            continue;
                        }
#endif
                        if (const auto expl = runtimeData.explosion) {
                            if (pFlags & (1 << 15)) {   //  kDestroyAfterHit flag required, otherwise the explosion will cast every moment
                                if (!(pFlags & 0u/*(1 << 31)*/)) {  // I don't know why but causing crashes without kMoved flag
                                    if (expl->GetFormID()) spdlog::debug("{}. draupnir explosion is: {:08x}", i, expl->formID);
                                    else spdlog::debug("{}. draupnir explosion not has formID", i);
                                //  if (!(pFlags & (1 << 22))) pFlags |= (1 << 22); //  i tried it for make working the explosions after hitting living targets, but not worked
                                //  if (pFlags & (1 << 13)) pFlags &= ~(1 << 13);
                                //  if (pFlags & (1 << 25)) pFlags &= ~(1 << 25);
                                //      spdlog::debug("{}. Draupnir updated runtime flags: {:032b}", i, pFlags);
                                    expl->data.damage   = a_damage;
                                    expl->data.force    = a_force;

                                    if (!(pFlags & (1 << 2)) && !(pFlags & (1 << 22)) && (pFlags & (1 << 21)) && (pFlags & (1 << 25))) {    //  means hitted to a living target
                                        spdlog::debug("!!experimental debug 2");
                                        if (!SpellDraupnirsCallProjL) continue;
#ifdef EXPERIMENTAL
                                    //  experimental
                                        const auto AnArchos = RE::PlayerCharacter::GetSingleton();
                                        for (auto hitBone : DraupnirSpearHitBones) {
                                            auto target = DraupnirSpearHitActors[i - 1];
                                            if (hitBone && target) {
                                                const float dist = 6969.f;
                                                const float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.1f);
                                                const auto delay = randomFloat + static_cast<float>(j) / 4.f;
                                                DraupnirsCallProjBaseL->data.speed = dist / delay;

                                                RE::ProjectileHandle pHandle;
                                                RE::NiPoint3 startPoint = hitBone->world.translate + RE::NiPoint3(0.f, 0.f, dist);
                                                RE::Projectile::ProjectileRot pRot = {0.f, 0.f};    //{AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
                                                RE::Projectile::LaunchSpell(&pHandle, AnArchos, SpellDraupnirsCallProjL, startPoint, pRot);
                                                proj = nullptr;

                                                spdlog::debug("!!experimental debug 2.1");
                                            //  std::jthread delayedCast([=]() {
                                            //      DelayedCast(AnArchos, SpellDraupnirsCallProjL, hitBone, delay);
                                            //      target->RemoveExtraArrows3D();
                                            //      DraupnirSpearHitBones[i - 1] = nullptr;
                                            //      DraupnirSpearHitActors[i - 1] = nullptr;
                                            //  });
                                            //  delayedCast.detach();
                                            }
                                        }
                                        j++;
#endif
                                        continue;
                                    }

                                    if (runtimeData.explosionTimer > 0.f) {
                                        if (!(pFlags & (1 << 2)) && !(pFlags & (1 << 20))) runtimeData.explosionTimer = 0.02f;  //  explodes immediately if is not stucked anywhere
                                        else {
                                        //  float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.1f);
                                            runtimeData.explosionTimer = static_cast<float>(j) / 4.f;// + randomFloat;
                                        } j++;  //  update explodable spear counter
                                    } else spdlog::info("{}. Draupnir not has explosion timer", i);

                                    if (const auto formID = proj->GetFormID()) spdlog::info("{}. Draupnir Spear's ({:08x}) explositon timer is: {}", i, formID, runtimeData.explosionTimer);
                                    else {spdlog::debug("{}. Draupnir Spear's (null) explositon timer is: {}", i, runtimeData.explosionTimer);}
                                } else {spdlog::debug("{}. Draupnir is not exist", i);}
                            } else {spdlog::debug("{}. Draupnir is not destructable (runtimeFlag)", i);}
                        } else {spdlog::info("{}. Draupnir not has explosion", i);}
                    } else {spdlog::debug("{}. Draupnir is kStillLoading (formFlag)", i);}
                } else {spdlog::debug("{}. Draupnir is kDisabled (formFlag)", i);}
            } else {spdlog::debug("{}. Draupnir is not inited (formFlag)", i);}
        } else {spdlog::debug("{}. Draupnir Spear is nullptr", i);}
    }
/*
#ifdef EXPERIMENTAL
//  experimental
    if (!SpellDraupnirsCallProjL) return;
    const auto AnArchos = RE::PlayerCharacter::GetSingleton();
    int k = 0;
    int l = 1;
    for (auto hitBone : DraupnirSpearHitBones) {
        const auto proj = DraupnirSpearProjectiles[k];
        const auto target = DraupnirSpearHitActors[k];
        if (proj && hitBone && target) {
            const float dist = 6969.f;
            const float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.1f);
            const auto delay = randomFloat + static_cast<float>(j) / 4.f;
            DraupnirsCallProjBaseL->data.speed = dist / delay;

            RE::ProjectileHandle pHandle;
            RE::NiPoint3 startPoint = hitBone->world.translate + RE::NiPoint3(0.f, 0.f, dist);
            RE::Projectile::ProjectileRot pRot = {0.f, 0.f};    //{AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
            RE::Projectile::LaunchSpell(&pHandle, AnArchos, SpellDraupnirsCallProjL, startPoint, pRot);

            hitBone = nullptr;
            DraupnirSpearProjectiles[k] = nullptr;
            DraupnirSpearHitActors[k] = nullptr;
        //  const float randomFloat = MathUtil::Algebra::generateRandomFloat(-0.1f, 0.1f);
        //  const auto delay = randomFloat + static_cast<float>(l) / 4.f;
        //  l++;
        //  std::jthread delayedCast([=]() {
        //      DelayedCast(AnArchos, SpellDraupnirsCallProjL, hitBone, delay);
        //      target->RemoveExtraArrows3D();
        //  });
        //  delayedCast.detach();
        } else spdlog::debug("!!experimental debug 3");
        k++;
    }
#endif
*/
}

using EventChecker = RE::BSEventNotifyControl;
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
        case "CallWeapon"_h:
            if (auto Levi = LeviathanAxe::GetSingleton())
        //      Levi->GetThrowState() == tState::kCanArrive
        //   || Levi->GetThrowState() == tState::kThrown)
                    Levi->Call();
        //  else spdlog::warn("!!the ax did not come");
            break;
        case "CatchLevi"_h:
            WeaponIdentify::WeaponCheck();
            if (WeaponIdentify::LeviathanAxe) {
                auto AnArchos = PlayerCharacter::GetSingleton();
                auto Levi = LeviathanAxe::GetSingleton();
                if (AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe) {
                    Levi->Catch(Levi->LeviathanAxeProjectileA, AnArchos);
                }
            } else spdlog::warn("Levi is not catched!!");
            break;
        case "LeviCallAttack"_h:    //event: attackPowerStartInPlace, attackStart, PowerAttack [IDLE:000E8456], NormalAttack [IDLE:00013215]
            if (WeaponIdentify::LeviathanAxe) {
                auto AnArchos = PlayerCharacter::GetSingleton();
                auto eqManager = RE::ActorEquipManager::GetSingleton();
                auto Levi = LeviathanAxe::GetSingleton();
            //  auto eqEntryData = AnArchos->GetEquippedEntryData(false);
                if (AnArchos && Levi && eqManager) {
                //  Levi->Call();
                //  auto eqExtraList = eqEntryData->extraLists ? eqEntryData->extraLists->front() : nullptr;
                //  auto xData = static_cast<RE::ExtraFlags*>(eqExtraList->GetByType(RE::ExtraDataType::kFlags));
                //  AnArchos->GetActorRuntimeData().boolBits.set(RE::Actor::BOOL_BITS::kSwimming);
                    eqManager->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe, nullptr, 1U, nullptr, true, true, false, true);
                //  AnArchos->GetActorRuntimeData().boolBits.reset(RE::Actor::BOOL_BITS::kSwimming);
                }
            }
            else spdlog::warn("Levi is not callable");
            break;
        case "ThrowAttackStart"_h:
            if (auto kratos = Kratos::GetSingleton(); auto AnArchos = RE::PlayerCharacter::GetSingleton()) {
                if (WeaponIdentify::isLeviathanAxe)
                    kratos->SetIsCanCharge(AnArchos, false);
            }
            break;
        case "ThrowWeapon"_h:
            if (auto Levi = LeviathanAxe::GetSingleton(); Levi->GetThrowState() == tState::kThrowable) {
                Levi->Throw(false);
            }
            else spdlog::warn("Levi is not throwable");
            break;
        case "ThrowWeaponV"_h:
            if (auto Levi = LeviathanAxe::GetSingleton(); Levi->GetThrowState() == tState::kThrowable) {
                Levi->Throw(true);
            }
            else spdlog::warn("Levi is not throwable");
            break;
        case "ThrowAttackEnd"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                auto AnArchos = PlayerCharacter::GetSingleton();
                if (auto eqManager = RE::ActorEquipManager::GetSingleton(); auto eqEntryData = AnArchos->GetEquippedEntryData(false)) {
                    if (auto eqExtraList = eqEntryData->extraLists) {
                        auto eqExtraListRef = eqExtraList->empty() ? nullptr : eqExtraList->front();
                        WeaponIdentify::skipEquipAnim = true;
                        eqManager->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe, eqExtraListRef, 1U, nullptr, false, true, false, true);
                    }
                }
            }
            break;
        case "LeviChargeStart"_h:
            if (auto kratos = Kratos::GetSingleton(); auto AnArchos = RE::PlayerCharacter::GetSingleton()) {
                if (auto handEffect = kratos->VFXeffect.handFrost)
                    AnArchos->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone, false);
                if (auto soundEffect = kratos->soundEffect.chargeLevi)
                    ObjectUtil::Sound::PlaySound(soundEffect, WeaponIdentify::RHandBone, 5.f);
            }
            break;
        case "LeviCharge"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos->IsCanCharge(PlayerCharacter::GetSingleton()))
                LeviathanAxe::GetSingleton()->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, -1);
            else spdlog::info("still in charging cooldown time");
            break;
        case "ThrowSpear"_h:
            Draupnir::Throw();
            break;
        case "DraupnirsCall"_h:
            if (WeaponIdentify::DraupnirSpear) {
                const float draupnirDamage = WeaponIdentify::DraupnirSpear->attackDamage;
                const float damage = draupnirDamage * WeaponIdentify::DamageMult;
                Draupnir::Call(damage, draupnirDamage);
            } else Draupnir::Call(10.f, 7.f);
            break;
        //  rage
        case "SpartanRageTriggerStart"_h:
            break;
        case "SpartanRageTriggerEnd"_h:
            break;
#ifdef EXPERIMENTAL_SHIELD
        //  animated shield
        case "shieldOpen"_h:
            if (auto kratos = Kratos::GetSingleton()) kratos->OpenShield(PlayerCharacter::GetSingleton());
            break;
        case "shieldClose"_h:
            if (auto kratos = Kratos::GetSingleton()) kratos->CloseShield(PlayerCharacter::GetSingleton());
            break;
        case "MCO_Recovery"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos->IsShieldOpened()) kratos->CloseShield(PlayerCharacter::GetSingleton());
            break;
#endif
        case "weaponDraw"_h:
            WeaponIdentify::WeaponCheck();
            if (WeaponIdentify::LeviathanAxe) {
                auto AnArchos = PlayerCharacter::GetSingleton();
                auto Levi = LeviathanAxe::GetSingleton();
                if (AnArchos->GetEquippedObject(false) == WeaponIdentify::LeviathanAxe) {
                    if (WeaponIdentify::LeviathanAxe->HasWorldModel()) {
                        spdlog::debug("Levi is throwable");
                        Levi->SetThrowState(tState::kThrowable);
                        Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, true);
                    } else spdlog::debug("Levi is not equipped for real!!");
                }
            }
            break;
        case "weaponSwing"_h:
            if (WeaponIdentify::isLeviathanAxe)
                if (auto Levi = LeviathanAxe::GetSingleton()) {
                    Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag);
                }
            break;
    //  case "CastOKStart"_h:
    //  case "MCO_AttackInitiate"_h:
    //  case "MCO_PowerAttackInitiate"_h:
    //  case "TDM_AttackStart"_h:
    //  case "Collision_AttackStart"_h:     //  axe calling attack state
    //      if (!WeaponIdentify::isRelic && AnArchos->IsAttacking() && SpellCastTracker::EffectAimButton && AnArchos->AsMagicTarget()->HasMagicEffect(SpellCastTracker::EffectAimButton)) {
    //          RE::ActorEquipManager::GetSingleton()->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, false, false, false, true);
    //          AnArchos->NotifyAnimationGraph("MCO_AttackInitiate");
    //      }
    //      break;
        }
    }
        return EventChecker::kContinue;
}
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
            auto levi = LeviathanAxe::GetSingleton();
            auto kratos = Kratos::GetSingleton();
            if (!caster || !levi || !kratos) return EventChecker::kContinue;

            if (formID == kratos->spellID.aim) {
                spdlog::debug("aiming...");
            } else if (formID == kratos->spellID.call) {
                if (!WeaponIdentify::isRelic) {
                    if (levi->GetThrowState() == tState::kCanArrive) {
                        if (levi->data.weap) {
                            ObjectUtil::Actor::EquipItem(caster, levi->data.weap->formID, false, 1u, true, false, false);
                        } else spdlog::debug("levi does not exist!");
                    } else spdlog::debug("levi can't arrive!");
                } else if (WeaponIdentify::isLeviathanAxe) {
                    if (!levi->isAxeThrowed && kratos->IsCanCharge(caster, Kratos::Relic::kLeviathanAxe))
                        caster->NotifyAnimationGraph("IdleCombatStart");
                } else if (WeaponIdentify::isBladeOfChaos) {
                    if (kratos->IsCanCharge(caster, Kratos::Relic::kBladeOfChaos))
                        caster->NotifyAnimationGraph("IdleCombatStart");
                } else if (WeaponIdentify::isDraupnirSpear) {
                    caster->NotifyAnimationGraph("IdleCombatStart");
                }
            } else if (formID == kratos->spellID.runic) {
                if (caster->HasSpell(kratos->SpellFinisherButton)) {
                    if (kratos->IsCanRage(caster)) {
                        kratos->StartRage(caster, Kratos::Rage::kSpartan);
                    } else if (caster->HasSpell(kratos->SpellSpartanRage)) {
                        kratos->EndRage(caster);
                    } else spdlog::debug("you cant rage 1");
                }
            } else if (formID == kratos->spellID.finisher) {
                if (caster->HasSpell(kratos->SpellRunicButton)) {
                    if (kratos->IsCanRage(caster)) {
                        kratos->StartRage(caster, Kratos::Rage::kSpartan);
                    } else if (caster->HasSpell(kratos->SpellSpartanRage)) {
                        kratos->EndRage(caster);
                    } else spdlog::debug("you cant rage 2");
                }
            } else if (formID == kratos->spellID.leviChargeCoolDown) {
                spdlog::debug("levi charge in cooldown...");
            }
        }
    }   return EventChecker::kContinue;
}
/*
//  check spell cast events
EventChecker InputEventTracker::ProcessEvent(RE::InputEvent* const *a_event, RE::BSTEventSource<RE::InputEvent*> *a_eventSource)
{
    if (a_event || RE::UI::GetSingleton()->GameIsPaused()) return EventChecker::kContinue;
    auto event = *a_event;
    if (event->HasIDCode() || event->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) return EventChecker::kContinue;

    auto keyCode = event->AsIDEvent()->GetIDCode();
    if (AimButton <= 0u || keyCode <= 0u) return EventChecker::kContinue;

    auto device = event->device.get();
    uint32_t aimButton = AimButton;
    switch (device) {
    case RE::INPUT_DEVICE::kKeyboard:
        break;
    case RE::INPUT_DEVICE::kGamepad:
        if (aimButton <= 266u) aimButton += 266u;
        break;
    case RE::INPUT_DEVICE::kMouse:
        if (aimButton <= 256u) aimButton += 256u;
        break;
    default:
        return EventChecker::kContinue;
    }

    if (keyCode == aimButton) {
        auto AnArchos = RE::PlayerCharacter::GetSingleton();
        if (APIs::tdm && APIs::tdm->GetDirectionalMovementState()) {
            APIs::result =  APIs::tdm->RequestDisableDirectionalMovement(SKSE::GetPluginHandle());
            if (APIs::result == TDM_API::APIResult::OK || APIs::result == TDM_API::APIResult::AlreadyGiven) spdlog::debug("Request TDM API ok");
            if (APIs::result != TDM_API::APIResult::OK && APIs::result != TDM_API::APIResult::AlreadyGiven) spdlog::debug("Request TDM API not ok");
            AnArchos->NotifyAnimationGraph("PickNewIdle");
            spdlog::info("aiming...");
                return EventChecker::kContinue;
        }
        if (APIs::tdm && !APIs::tdm->GetDirectionalMovementState()) {
            APIs::result =  APIs::tdm->ReleaseDisableDirectionalMovement(SKSE::GetPluginHandle());
            if (APIs::result == TDM_API::APIResult::OK || APIs::result == TDM_API::APIResult::AlreadyGiven) spdlog::debug("Release TDM API ok");
            if (APIs::result != TDM_API::APIResult::OK && APIs::result != TDM_API::APIResult::AlreadyGiven) spdlog::debug("Release TDM API not ok");
            AnArchos->NotifyAnimationGraph("PickNewIdle");
                return EventChecker::kContinue;
        }
    }   return EventChecker::kContinue;
}

void InputEventTracker::Register() {
            spdlog::info("input device manager registering...");
    auto sourceHolder = RE::BSInputDeviceManager::GetSingleton(); 
    if (sourceHolder) {
        sourceHolder->AddEventSink(InputEventTracker::GetSingleton());
            spdlog::info("input event sink registered!");
    } else  spdlog::warn("input event sink not registered!");
};
*/
/*
//  check spell cast events
EventChecker SpellCastTracker::ProcessEvent(const RE::TESSpellCastEvent* a_event, RE::BSTEventSource<RE::TESSpellCastEvent>* a_eventSource)
{
    if (a_event) {
        spdlog::debug("spell casted");
        auto sp = a_event->spell;
        if (sp) {
            if (sp == SpellAimButton->GetFormID()) {
                auto AnArchos = RE::PlayerCharacter::GetSingleton();
                if (EffectAimButton && AnArchos->AsMagicTarget()->HasMagicEffect(EffectAimButton)) {
                    if (APIs::tdm && APIs::tdm->GetDirectionalMovementState()) {
                        APIs::result =  APIs::tdm->RequestDisableDirectionalMovement(SKSE::GetPluginHandle());
                        if (APIs::result == TDM_API::APIResult::OK || APIs::result == TDM_API::APIResult::AlreadyGiven) spdlog::debug("Request TDM API ok");
                        if (APIs::result != TDM_API::APIResult::OK && APIs::result != TDM_API::APIResult::AlreadyGiven) spdlog::debug("Request TDM API not ok");
                        AnArchos->NotifyAnimationGraph("PickNewIdle");
                        spdlog::info("aiming...");
                            return EventChecker::kContinue;
                    }
                }
                if (EffectAimButton && !AnArchos->AsMagicTarget()->HasMagicEffect(EffectAimButton)) {
                    if (APIs::tdm && !APIs::tdm->GetDirectionalMovementState()) {
                        APIs::result =  APIs::tdm->ReleaseDisableDirectionalMovement(SKSE::GetPluginHandle());
                        if (APIs::result == TDM_API::APIResult::OK || APIs::result == TDM_API::APIResult::AlreadyGiven) spdlog::debug("Release TDM API ok");
                        if (APIs::result != TDM_API::APIResult::OK && APIs::result != TDM_API::APIResult::AlreadyGiven) spdlog::debug("Release TDM API not ok");
                        AnArchos->NotifyAnimationGraph("PickNewIdle");
                            return EventChecker::kContinue;
                    }
                }
            }
        }
    }   return EventChecker::kContinue;
}

void SpellCastTracker::Register() {
    auto sourceHolder = RE::ScriptEventSourceHolder::GetSingleton(); 
    if (sourceHolder) {
        sourceHolder->AddEventSink(SpellCastTracker::GetSingleton());
            spdlog::info("Spell cast event sink registered!");
    } else  spdlog::warn("Spell cast event sink not registered!");
};
*/