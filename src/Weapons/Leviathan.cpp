#include "Leviathan.h"
#include "RelicManager.h"

using namespace Util;

LeviathanAxe::LeviathanAxe(RE::TESBoundObject* a_object)
    : SmartRelicWeapon(a_object)
{
}

bool LeviathanAxe::Initialize()
{
    bool found = true;
    auto dataHandler    = RE::TESDataHandler::GetSingleton();
    LeviProjBaseL       = dataHandler->LookupForm<RE::BGSProjectile>   (0x815, Config::KratosCombatESP);
    LeviProjBaseH       = dataHandler->LookupForm<RE::BGSProjectile>   (0x816, Config::KratosCombatESP);
    LeviProjBaseA       = dataHandler->LookupForm<RE::BGSProjectile>   (0x822, Config::KratosCombatESP);
    SpellLeviProjL      = dataHandler->LookupForm<RE::SpellItem>       (0x811, Config::KratosCombatESP);
    SpellLeviProjH      = dataHandler->LookupForm<RE::SpellItem>       (0x813, Config::KratosCombatESP);
    SpellCatchLevi      = dataHandler->LookupForm<RE::SpellItem>       (0x81D, Config::KratosCombatESP);
    SpellLeviProjA      = dataHandler->LookupForm<RE::SpellItem>       (0x823, Config::KratosCombatESP);
    EnchCharge          = dataHandler->LookupForm<RE::EnchantmentItem> (0x84B, Config::KratosCombatESP);    // +30 frost damage, ff, touch
    if (LeviProjBaseL && LeviProjBaseH && LeviProjBaseA)
            spdlog::debug("Leviathan Axe projectiles are {}, {} and {}", LeviProjBaseL->GetName(), LeviProjBaseH->GetName(), LeviProjBaseA->GetName());
    else     {spdlog::error("Can't find Leviathan Axe projectiles"); found = false;}
    if (SpellLeviProjL && SpellLeviProjH)   
            spdlog::debug("Leviathan Axe throwing spells are {} and {}", SpellLeviProjL->GetName(), SpellLeviProjH->GetName());
    else     {spdlog::error("Can't find Leviathan Axe projectile spells"); found = false;}
    if (SpellLeviProjA)
            spdlog::debug("Leviathan Axe calling spell is {}", SpellLeviProjA->GetName());
    else     {spdlog::error("Can't find Leviathan Axe calling spell"); found = false;}
    if (SpellCatchLevi) {
            EffCatchLevi = SpellCatchLevi->effects[0]->baseEffect;
            spdlog::debug("Leviathan Axe catching spell is {}", SpellCatchLevi->GetName()); EffCatchLevi = SpellCatchLevi->effects[0]->baseEffect;}
    else     {spdlog::error("Can't find Leviathan Axe catching spell"); found = false;}
    if (EnchCharge) {           
            spdlog::debug("Default Leviathan Axe charging enchantment is {}", EnchCharge->GetName());}
    else     {spdlog::error("Can't find default Leviathan Axe enchantment spell"); found = false;}

    return found;
}
void LeviathanAxe::Update() {
    return;
    if (projectileUpdate.IsTimeToUpdate()) {
        if (projectileModel && projectile && projectile->Get3D() && weaponModelCopy && projectileModel == projectile->Get3D()) {
            const RE::BSFixedString rotatingBoneName = "BlastRadiusNode";
            auto animatedBone = projectileModel->GetObjectByName(rotatingBoneName);
            auto animatedNode = animatedBone ? animatedBone->AsNode() : nullptr;

            auto cloneModel = weaponModelCopy.get()->Clone();
            auto cloneNode = cloneModel ? cloneModel->AsNode() : nullptr;
            replacedProjectileModel.reset(cloneNode);

            if (animatedNode) {
                animatedNode->AttachChild(replacedProjectileModel.get(), false);
            //    auto oldWorld = transformW;
            //    oldWorld.translate *= 70.f;
            //    oldWorld.scale = replacedProjectileModel.get()->world.scale;
            //    replacedProjectileModel.get()->local = ObjectUtil::Node::GetLocalTransform(replacedProjectileModel.get(), oldWorld);
                projectileUpdate.Done();
                trailUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);
                spdlog::debug("levi projectileModel changed!");
            } else spdlog::warn("animated node null");
        } else spdlog::warn("projectile or projectile->Get3D2() null");
    }
    if (soundData.arrivingLoopStopUpdate.IsTimeToUpdate()) {soundData.StopArrivingLoopSounds();}
    if (soundData.throwingLoopStopUpdate.IsTimeToUpdate()) {soundData.StopThrowingLoopSounds();}
    if (Config::DrawTrails) {
        AddTrail();
        FadeTrail();
    }
//    if (replacedProjectileModel) {
//        auto projNiTransform = replacedProjectileModel->world;
//        auto projBHKTransform = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(replacedProjectileModel.get());
//        auto projHKPTransform = ObjectUtil::Node::GetHavokHKPRigidBodyWorldTransform(replacedProjectileModel.get());
//        spdlog::debug(
//            "NI: ({}, {}, {})  BHK: ({}, {}, {})",
//            projNiTransform.translate.x,
//            projNiTransform.translate.y,
//            projNiTransform.translate.z,
//            projBHKTransform.translate.x * 70.f,
//            projBHKTransform.translate.y * 70.f,
//            projBHKTransform.translate.z * 70.f
//        );
    //    spdlog::debug(
    //        "NI - BHK: ({}, {}, {})",
    //        projNiTransform.translate.x - projBHKTransform.translate.x * 70.f,
    //        projNiTransform.translate.y - projBHKTransform.translate.y * 70.f,
    //        projNiTransform.translate.z - projBHKTransform.translate.z * 70.f
    //    );
//    }
}

void LeviathanAxe::SetState(RelicWeaponState::Type a_type)
{
    switch (a_type)
    {
    case RelicWeaponState::Type::kArriving:
        {
            auto previous = dynamic_cast<LeviathanArrivingState*>(currentState.get());
            auto rHandBone = thrower->GetRHandBone();
            RE::NiPoint3 startPoint; GetPosition(startPoint);
            if (isPenetrating && previous) {
                SmartRelicWeapon::SetState(std::make_unique<LeviathanArrivingState>(*previous, startPoint));
            } else {
                SmartRelicWeapon::SetState(std::make_unique<LeviathanArrivingState>(*this, startPoint, &rHandBone));
                spdlog::debug("Levi call is started");
            }
        }
        break;
    case RelicWeaponState::Type::kHoming:
        SmartRelicWeapon::SetState(std::make_unique<LeviathanHomingState>(*this));
        break;

    default:
        break;
    }
}
void LeviathanAxe::OnMenuOpenCloseEvent(const bool a_opening)
{
    if (a_opening) {
        soundData.PauseAllLoopingSounds();
    } else {
        soundData.ContinueAllLoopingSounds();
    }
}
/*
void LeviathanAxe::GetPosition(RE::NiPoint3& a_point)
{
    if (replacedProjectileModel) {
        transformW = GetWorldTransform();
        a_point = transformW.translate * 70.f;
    //    a_point = replacedProjectileModel->worldBound.center;
        spdlog::debug("leviathan coming from weapon projectileModel location.");
    } else spdlog::debug("we can't get leviathan's weapon projectileModel!");

    if (stuckedBone) {
        a_point = replacedProjectileModel ? transformW.translate : stuckedBone->world.translate;
        stuckedBone.reset();

        if (stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
            ObjectUtil::Projectile::DeleteAnExtraArrow(stuckedActor, projectileModel);
#else
            stuckedActor->RemoveExtraArrows3D();
#endif
            spdlog::debug("levi stucked actor's extra arrows removed");
            stuckedActor.reset();
        } else spdlog::debug("levi not stucked anybody");
    } else spdlog::debug("levi not stucked any bone");

    auto throwerActor = thrower ? thrower->GetActor() : nullptr;
    if (!throwerActor) return;

    if (GetThrowState() == ThrowState::kThrowable) {
        if (auto backWeaponSheathe = throwerActor->GetNodeByName("WeaponBack"); backWeaponSheathe) {
            const auto& backSheatheTransform = backWeaponSheathe->world;
            a_point = backSheatheTransform.translate;
            const auto& rightDir = backSheatheTransform.rotate * rightVec3;
            const auto& backDir = backSheatheTransform.rotate * backVec3;
            velocity = (0.69f * rightDir + 0.31f * backDir) * 2400.f;
            spdlog::debug("levi is coming from your back sheathe");
        }
    } else {
        auto pcPos = throwerActor->GetPosition();
        float dist = pcPos.GetDistance(a_point);
        if (dist > 36000.f) {   // ~42000 is limit
            spdlog::info("levi is too far away from you! ({} m)", (int)dist / 100);
            auto dir = a_point - pcPos;
            dir.Unitize();
            a_point = pcPos + dir * 36000.f;
        }
    }
}
*/
/*
void LeviathanAxe::Throw(const bool a_isVertical, const bool isPenetrating, const bool isHoming)
{
    auto rHandBone = thrower->GetRHandBone();
    if (!rHandBone) {spdlog::error("LeviathanAxe::Throw - RHandBone is null"); return;}

    auto throwerActor = thrower ? thrower->GetActor() : nullptr;
    if (!throwerActor) {spdlog::error("LeviathanAxe::Throw - thrower actor is null"); return;}

    trailRemoveUpdate.Done();

    bool isLeviathanAxe = thrower->GetRightHandRelic()->GetType() == RelicType::kLeviathanAxe;
    bool isVertical = a_isVertical;
    bool isThrowAttack = false;
    bool isPowerThrowAttack = false;
    if (Config::IsAdvancedThrowingInstalled) {
        throwerActor->GetGraphVariableBool("bIsThrowing", isThrowAttack);
        throwerActor->GetGraphVariableBool("bIsPowerThrowing", isPowerThrowAttack);
        isVertical = isPowerThrowAttack;
    }
    const auto leviThrowSpell = (a_isVertical || isVertical ? SpellLeviProjH : SpellLeviProjL);
//  auto leviBaseProj = (isVertical ? LeviProjBaseH : LeviProjBaseL);
    if (leviThrowSpell && (isLeviathanAxe || isPenetrating)) 
    {   //  calculate damage
        const auto leviProjEff = leviThrowSpell->effects[0];
        auto& leviProjEffSetting = leviProjEff->effectItem;
        auto& projectileDamage = leviProjEffSetting.magnitude;
        const auto leviDamage = damage;
        projectileDamage = leviDamage * thrower->GetDamageMult() * Config::ThrowingDamageMult;
        bool isPowerThrow; throwerActor->GetGraphVariableBool("IsPowerThrowing", isPowerThrow);
        if (isVertical || isPowerThrow) {projectileDamage *= 1.5f; yAngle = 1.57f;}
        else yAngle = 0.35f;
        float throwChargeDamageMult = std::sqrtf(throwingChargeDuration + 1.f);
        if (throwChargeDamageMult > 2.f) throwChargeDamageMult = 2.f;
        projectileDamage *= throwChargeDamageMult;

        if (const auto leviProjBaseEff = leviProjEff->baseEffect; leviProjBaseEff && leviProjBaseEff->data.projectileBase) {
        //  //  leviProjBaseEff->projectileBase->defaultWeaponSource = WeaponIdentify::LeviathanAxe;
        //  //  leviProjBaseEff->associatedForm = WeaponIdentify::LeviathanAxe;
            auto& pbData = leviProjBaseEff->data.projectileBase->data;
            pbData.speed = !isPenetrating ? Config::ThrowSpeed * std::clamp(throwChargeDamageMult / 2.f, 1.f, 1.25f) : pbData.speed * 0.7f;
            pbData.force = projectileDamage;
            pbData.gravity = 3.21f;
        } else spdlog::warn("not found Levi throwing effect!");

        if (!isPenetrating) {
            soundData.PlayThrowingSounds(rHandBone);

            gravity = 3.21f;
            gravity /= (std::powf(throwingChargeDuration + 1.f, 3.f));
            gravity = std::max(gravity, 0.5f);
        }

        //  set the launch data
        auto origin = isPenetrating ? position : rHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = {throwerActor->GetAimAngle(), throwerActor->GetAimHeading()};
        if (projectileModel && (isPenetrating)) throwerActor->Unk_A0(projectileModel, pRot.x, pRot.z, origin);
        RE::Projectile::LaunchData lData(throwerActor, origin, pRot, leviThrowSpell);

    //    lData.weaponSource = weap; // somehow causing very high damage
#ifdef EXPERIMENTAL_THROWPOISON
        lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(throwerActor, false);
#endif
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(throwerActor) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(throwerActor);
        else
            lData.enchantItem = nullptr;
        _isLastThrowCharged = lData.enchantItem != nullptr;
        //  throw the projectile
        projectileHandle = RE::Projectile::Launch(&pHandle, lData);
        projectile = pHandle.get().get();

        if (isLeviathanAxe) {
            const auto root = throwerActor->Get3D1(false);
            auto weapon3D = root ? root->GetObjectByName("WEAPON") : nullptr;
            auto copyWeaponModelObj = weapon3D ? weapon3D->Clone() : nullptr;
            auto copyWeaponModel = copyWeaponModelObj ? copyWeaponModelObj->AsNode() : static_cast<RE::NiAVObject*>(copyWeaponModelObj);
            if (copyWeaponModel) {
                copyWeaponModel->RemoveExtraData("BSX");
                copyWeaponModel->RemoveExtraData("BSXFlags");
                if (copyWeaponModel->GetCollisionObject())
                    copyWeaponModel->GetCollisionObject()->flags.reset(RE::bhkCollisionObject::Flag::kActive);
                copyWeaponModel->collisionObject.reset();
                auto copyWeaponModelNode = copyWeaponModel ? copyWeaponModel->AsNode() : nullptr;
                weaponModelCopy.reset(copyWeaponModelNode);
                if (weaponModelCopy) {
                    weaponModelCopy->local = RE::NiTransform();
                    weaponModelCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
                }
            }
        }

        projectileUpdate.RegisterForUpdate(0.0f, false);

        if (isHoming) {
        //    if (isPenetrating) {
        //        //
        //    } else {
                std::vector<RE::ActorHandle> nearCombatTargets = ObjectUtil::Actor::GetNearCombatTargetHandles<std::vector<RE::ActorHandle>>(throwerActor, Config::HProjectileTargetRange, true);
                SetState(std::make_unique<LeviathanHomingState>(*this, std::move(nearCombatTargets)));
        //    }
        }
        if (isPenetrating) return;

        if (Config::IsAdvancedThrowingInstalled && (isThrowAttack || isPowerThrowAttack)) {
            ResetCharge(enchMag, defaultEnchMag, true);
            thrower->SetSkipEquipAnim(true);
            ObjectUtil::Actor::UnEquipItem(throwerActor, false, false, true, true, thrower->GetSkipEquipAnim(), true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, throwerActor);
            spdlog::debug("Leviathan unequipped after throwing");
        } else {
    //        WeaponIdentify::isLeviathanAxe = false;
    //        WeaponIdentify::isRelic = false;
        //    Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
        //    throwerActor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            thrower->SetSkipEquipAnim(true);
            thrower->SetUnequipWhenAnimEnds(true);
        }

        throwerActor->SetGraphVariableBool("bLeviInCatchRange", false);

        isAxeCalled = false;
        isAxeThrowed = true;
        SetThrowState(ThrowState::kThrown);
            spdlog::info("Leviathan Axe throwed, raw damage is: {}", projectileDamage);
        if (stuckedBone)   stuckedBone    = nullptr;
        if (stuckedActor)  stuckedActor   = nullptr;
        lastHitActors.clear();
        lastHitForms.clear();
        if (throwerActor->HasSpell(SpellCatchLevi)) throwerActor->RemoveSpell(SpellCatchLevi);
    } else spdlog::info("Leviathan Axe is not equipped for throwing!");
}
*/
bool LeviathanAxe::PreThrow()
{
    trailRemoveUpdate.Done();

    bool isLeviathanAxe = thrower->GetRightHandRelic() == this;
    if (!isLeviathanAxe) {
        spdlog::debug("Leviathan Axe not equipped for throwing!");
        return false;
    }

    return true;
}
void LeviathanAxe::PostThrow()
{
    auto rHandBone = thrower->GetRHandBone();
    if (!rHandBone) {spdlog::error("LeviathanAxe::Throw - RHandBone is null"); return;}

    auto throwerActor = thrower ? thrower->GetActor() : nullptr;
    if (!throwerActor) {spdlog::error("LeviathanAxe::Throw - thrower actor is null"); return;}

    if (!projectile) {spdlog::error("LeviathanAxe::Throw - projectile is null"); return;}
    auto& projectileRTD = projectile->GetProjectileRuntimeData();

    {   //  calculate damage
        auto& projectileDamage = projectileRTD.weaponDamage;
        float throwChargeDamageMult = std::sqrtf(throwingChargeDuration + 1.f);
        if (throwChargeDamageMult > 2.f) throwChargeDamageMult = 2.f;
        projectileDamage *= throwChargeDamageMult;

        if (ThrowableWeaponDummyProjectile) {
        //  //  ThrowableWeaponDummyProjectile->defaultWeaponSource = weap;
            auto& pbData = ThrowableWeaponDummyProjectile->data;
            pbData.speed = !isPenetrating ? Config::ThrowSpeed * std::clamp(throwChargeDamageMult / 2.f, 1.f, 1.25f) : pbData.speed * 0.7f;
            pbData.force = projectileDamage;
            pbData.gravity = 3.21f;
        } else spdlog::warn("not found Levi throwing effect!");

        if (!isPenetrating) {
            soundData.PlayThrowingSounds(rHandBone);

            gravity = 3.21f;
            gravity /= (std::powf(throwingChargeDuration + 1.f, 3.f));
            gravity = std::max(gravity, 0.5f);
        }

        RE::EnchantmentItem* enchantItem = nullptr;
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(throwerActor) > 0.f)
            enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(throwerActor);
        else
            enchantItem = nullptr;

        _isLastThrowCharged = enchantItem != nullptr;

        projectileUpdate.RegisterForUpdate(0.0f, false);

        if (isPenetrating) return;

        if (Config::IsAdvancedThrowingInstalled) {
            ResetCharge(enchMag, defaultEnchMag, true);
            thrower->SetSkipEquipAnim(true);
            ObjectUtil::Actor::UnEquipItem(throwerActor, false, false, true, true, thrower->GetSkipEquipAnim(), true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, throwerActor);
            spdlog::debug("Leviathan unequipped after throwing");
        } else {
    //        WeaponIdentify::isLeviathanAxe = false;
    //        WeaponIdentify::isRelic = false;
        //    Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
        //    throwerActor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            thrower->SetSkipEquipAnim(true);
            thrower->SetUnequipWhenAnimEnds(true);
        }

        throwerActor->SetGraphVariableBool("bLeviInCatchRange", false);

        isAxeCalled = false;
        isAxeThrowed = true;
        SetThrowState(ThrowState::kThrown);
            spdlog::info("Leviathan Axe throwed, raw damage is: {}", projectileDamage);
        if (stuckedBone)   stuckedBone    = nullptr;
        if (stuckedActor)  stuckedActor   = nullptr;
        lastHitActors.clear();
        lastHitForms.clear();
        if (throwerActor->HasSpell(SpellCatchLevi)) throwerActor->RemoveSpell(SpellCatchLevi);
    }
}

void LeviathanAxe::Call(Caller* a_caller, const bool a_justDestroy, std::optional<float> a_delay)
{
    SmartRelicWeapon::Call(a_caller, a_justDestroy, a_delay);
    return;
    caller = a_caller;
    if (caller && caller->IsValid() && weap) {
        spdlog::debug("Levi is calling...");
        projectileUpdate.Done();

        trailUpdate.Done();
        trailRemoveUpdate.Done();

        if (projectileModel) {
            transformPW = projectileModel->world;
            transformPL = projectileModel->local;
        }
        projectileModel = nullptr;
    //    if (replacedProjectileModel) {
    //        if (projState == ProjectileState::kHavok && replacedProjectileModel->collisionObject && replacedProjectileModel->collisionObject->AsBhkRigidBody()) {
    //            RE::hkTransform rbTransform;
    //            replacedProjectileModel->collisionObject->AsBhkRigidBody()->GetTransform(rbTransform);
    //            RE::NiTransform niTransform;
    //            niTransform.translate = MathUtil::Algebra::HkVectorToNiPoint(rbTransform.translation);
    //            niTransform.rotate = MathUtil::Algebra::HKMatrixToNiMatrix(rbTransform.rotation);
    //            transformW = niTransform;
    //            transformL = niTransform;
    //        } else {
    //            transformW = replacedProjectileModel->world;
    //            transformL = replacedProjectileModel->local;
    //        }
    //    }

        soundData.FadeThrowingLoopSounds(369);

        auto stuckedLevi =  LastLeviProjectile ? LastLeviProjectile : nullptr;
        if (!stuckedLevi)   stuckedLevi = (LeviathanAxeProjectileL ? LeviathanAxeProjectileL : (LeviathanAxeProjectileH ? LeviathanAxeProjectileH : nullptr));
        if (stuckedLevi) {
            if (!isPenetrating) position = stuckedLevi->data.location;
            auto& projectileRTD = stuckedLevi->GetProjectileRuntimeData();
            auto& pFlags = projectileRTD.flags;
            if (!(pFlags & pFlag::kDestroyed)) {
                pFlags |= pFlag::kDestroyed;
            } else  spdlog::debug("levi is already destroyed");

            if (a_justDestroy) {
                isAxeCalled = false;
                isAxeThrowed = false;
                if (stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
                    ObjectUtil::Projectile::DeleteAnExtraArrow(stuckedActor, projectileModel);
#else
                    stuckedActor->RemoveExtraArrows3D();
#endif
                    spdlog::debug("levi stucked actor's extra arrows removed");
                    stuckedActor = nullptr;
                } else spdlog::debug("levi not stucked anybody");
                    return;
            }
        } else {spdlog::debug("Stucked Levi is nullptr!");}

        if (auto AnArchos = caller->GetActor(); !a_justDestroy && AnArchos && SpellLeviProjA) {
            isAxeCalled = true;
            isAxeThrowed = false;

            if (!Config::DontDamageWhileArrive) {
                const auto leviProjEff = SpellLeviProjA->effects[0];
                auto& leviProjEffSetting = leviProjEff->effectItem;
                auto& projectileDamage = leviProjEffSetting.magnitude;
                const auto leviDamage = damage;
                projectileDamage = leviDamage * caller->GetDamageMult() * Config::ThrowingDamageMult;
                projectileDamage *= 0.5f;
                if (const auto leviProjBaseEff = leviProjEff->baseEffect; leviProjBaseEff && leviProjBaseEff->data.projectileBase) {
                //    leviProjBaseEff->projectileBase->defaultWeaponSource = WeaponIdentify::LeviathanAxe;
                //    leviProjBaseEff->associatedForm = WeaponIdentify::LeviathanAxe;
                    auto& pbData = leviProjBaseEff->data.projectileBase->data;
                    pbData.force = projectileDamage * 2.f;
                } else spdlog::warn("not found Levi arriving effect!");
                spdlog::debug("damage mult: {} throwing dm {} levi damage {} total damage {}", caller->GetDamageMult(), Config::ThrowingDamageMult, leviDamage, projectileDamage);
            }

            RE::NiPoint3 startPoint = position;
            auto rHandBone = caller->GetRHandBone();
            RE::NiPoint3 targetPoint = rHandBone ? rHandBone->world.translate : AnArchos->GetPosition();
            if (!isPenetrating) {
                soundData.PlayCallingHandSounds(rHandBone);
                GetPosition(startPoint);
            }
            RE::ProjectileHandle pHandle;
            projectileRotation = MathUtil::Algebra::VectorToPitchYaw(direction);
            RE::Projectile::LaunchData lData(AnArchos, startPoint, projectileRotation, SpellLeviProjA);

            lData.noDamageOutsideCombat = true; //  can be an option
            lData.weaponSource = weap;
#ifdef EXPERIMENTAL_THROWPOISON
            lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(AnArchos, false);
#endif
            if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
                lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

            projectileHandle = RE::Projectile::Launch(&pHandle, lData);
            projectile = pHandle.get().get();

            projectileUpdate.RegisterForUpdate(0.0f, false);

            SetState(RelicWeaponState::Type::kArriving);

            SetThrowState(ThrowState::kArriving);
            spdlog::info("Levi is arriving...");
        } else {spdlog::warn("WEIRD SpellLeviProjA is nullptr!");}
    } else {spdlog::warn("WEIRD you don't have the axe for calling!!");}
}
void LeviathanAxe::Catch(const bool a_justDestroy)
{
    if (LeviathanAxeProjectileA) {
    //    if (APIs::precision || APIs::Request()) {
    //        APIs::precision->RemoveProjectileCollision(throwerActor->GetHandle(), collisionDefinition);
    //    }

        auto& runtimeData = LeviathanAxeProjectileA->GetProjectileRuntimeData();
        runtimeData.flags |= pFlag::kDestroyed;
        if (a_justDestroy) return;
    }

    auto callerActor = caller ? caller->GetActor() : nullptr;
    if (callerActor && !caller->GetRightHandRelic()) {
        callerActor->SetGraphVariableBool("bLeviInCatchRange", true);
        if (EffCatchLevi && SpellCatchLevi && !callerActor->AsMagicTarget()->HasMagicEffect(EffCatchLevi)) {
            callerActor->AddSpell(SpellCatchLevi);
        }

        SetThrowState(ThrowState::kArrived);

        auto assets = Assets::GetSingleton();
        auto rHandBone = caller->GetRHandBone();
        if (auto handEffect = assets->VFXeffects.handFrost; handEffect) 
            callerActor->ApplyArtObject(handEffect, 1.f, nullptr, false, false, rHandBone);

        soundData.FadeArrivingNearSounds(469);
    //    soundData.FadeArrivingLoopSounds(469);
        soundData.StopArrivingLoopSounds(*g_deltaTimeRealTime * 1200.f);
        soundData.PlayCatchingSounds(rHandBone);

        if (weap) {
            caller->SetSkipEquipAnim(true);
            caller->SetUnequipWhenAnimEnds(false);
            if (callerActor->IsPlayerRef())
                Config::SpecialWeapon->value = (uint8_t)RelicType::kNone;
            callerActor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            caller->DoAction(ActionType::kWeaponCharge);
            ObjectUtil::Actor::EquipItem(callerActor, weap, caller->GetSkipEquipAnim());//, 1U, true, false, false, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, callerActor);
            RE::ShakeCamera(0.3f, position, 0.5f);
            if (caller->GetSkipEquipAnim()) caller->SetSkipEquipAnim(false);
        } else spdlog::warn("you not have the leviathan axe");

        if (stuckedBone)   stuckedBone    = nullptr;
        if (stuckedActor)  stuckedActor   = nullptr;
        lastHitActors.clear();
        lastHitForms.clear();

        if (Config::UsePrecisionTrails) {
            if (caller->GetAnimObjectRBone() && caller->GetAnimObjectRBone()->AsNode() && replacedProjectileModel && replacedProjectileModel->parent) {
                caller->GetAnimObjectRBone()->AsNode()->AttachChild(replacedProjectileModel->parent);
                replacedProjectileModel->parent->local.translate = RE::NiPoint3();
                replacedProjectileModel->parent->local.rotate = replacedProjectileModel->parent->local.rotate * RE::NiMatrix3(PI2, 0.f, PI2);
            }
        }
        trailUpdate.Done();
        projectileModel = nullptr;
        trailRemoveUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);
    }
}
void LeviathanAxe::Charge(const uint8_t a_chargeHitCount, const float a_magnitude, const uint8_t a_stage, const uint8_t a_coolDown)
{
    auto assets     = Assets::GetSingleton();
    auto AnArchos   = RE::PlayerCharacter::GetSingleton();
    auto ench       = weap ? ObjectUtil::Enchantment::GetInventoryItemEnchantment(AnArchos, weap) : nullptr;
    auto enchEffect = ench ? ench->effects[0] : nullptr;
    auto enchBase   = enchEffect ? enchEffect->baseEffect : nullptr;
    const auto leviDam  = damage;
    const auto weapBone = wielder->GetWeaponBone();

    if (weap && chargeHitCount <= 0) {
        if (enchBase) {
            spdlog::debug("levi's enchantment is: {}", ench->GetName());
            if (enchBase->HasArchetype(RE::EffectSetting::Archetype::kDualValueModifier)
             || enchBase->HasArchetype(RE::EffectSetting::Archetype::kValueModifier)) {
                ResetCharge(enchMag, defaultEnchMag, false, true);
                auto& projectileDamage = enchEffect->effectItem.magnitude;
                enchMag = &projectileDamage;
                defaultEnchMag = projectileDamage;
                projectileDamage *= a_magnitude;
                chargeHitCount = a_chargeHitCount;
                ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 300.f);
                _isCharged = true;
                spdlog::debug("magnitude buffing from {} to: {}", projectileDamage / a_magnitude, projectileDamage);

                if (auto handEffect = assets->VFXeffects.handFrostBright; handEffect) AnArchos->ApplyArtObject(handEffect, a_chargeHitCount * 2, nullptr, false, false, weapBone);
                else spdlog::warn("can't found hand effect for levi charge!");

                if (auto soundEffect = assets->soundEffects.chargeLeviEnd; soundEffect) ObjectUtil::Sound::PlaySound(soundEffect, weapBone, 5.f);
            } else spdlog::debug("levi's enchantment is not expected archetype.");
#ifdef EXPERIMENTAL_CHARGE_LEVI
        } else {
            spdlog::debug("levi not has any enchantment, levi is enchanting...");
            if (EnchCharge) {
                ench = EnchCharge;
                enchEffect  = ench ? ench->effects[0] : nullptr;
                enchBase     = enchEffect ? enchEffect->baseEffect : nullptr;
                if (enchBase) {
                    ResetCharge(enchMag, defaultEnchMag, false, true);
                    ObjectUtil::Enchantment::EnchantEquippedWeapon(AnArchos, ench, 300.f, false, false);
                    ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 300.f);
                    _isCharged = true;

                //  AnArchos->GetActorRuntimeData().emotionType = RE::EmotionType::kAnger;
                //  AnArchos->GetActorRuntimeData().emotionValue = 100;

                    auto& enchCost = ench->data.costOverride;
                    auto& enchAmount = ench->data.chargeOverride;
                    auto& projectileDamage = enchEffect->effectItem.magnitude;
                    enchAmount = 500.f;
                    projectileDamage = a_magnitude * leviDam / 2;
                    enchCost = projectileDamage;
                    enchMag = nullptr;

                    chargeHitCount = a_chargeHitCount;
                    spdlog::info("levi charge frost damage buff is: {}", projectileDamage);

                    if (auto handEffect = assets->VFXeffects.handFrostBright; handEffect) AnArchos->ApplyArtObject(handEffect, a_chargeHitCount * 2, nullptr, false, false, weapBone);
                    else spdlog::warn("can't found hand effect for levi charge!");

                    if (auto soundEffect = assets->soundEffects.chargeLeviEnd; soundEffect) ObjectUtil::Sound::PlaySound(soundEffect, weapBone, 5.f);
                } else spdlog::error("can't find frost enchantment's base!!");
            } else spdlog::error("can't find frost enchantment!!");
#endif
        }
    } else spdlog::error("can't find levi for charging!!");
}
void LeviathanAxe::ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck, const bool a_justReset)
{
    auto assets = Assets::GetSingleton();
    if (a_justReset) {
        if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), weap) == EnchCharge) {
            ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), weap);
            _isCharged = false;
        }
    } else {
        if (chargeHitCount <= 0) {
            if (a_magnitude) *a_magnitude = a_defMagnitude;
            else {
                if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), weap) == EnchCharge)
                    ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), weap);
            }
            _isCharged = false;
        } else if (!a_justCheck) {chargeHitCount -= 1;}
    }
}
