#include "ThrowableRelicWeapon.h"
#include "Assets.h"
#include "Actors/Capabilities/Thrower.h"

using namespace MathUtil;
using namespace GameSettingUtil;

using pbType = RE::BGSProjectileData::Type;
using pbFlag = RE::BGSProjectileData::BGSProjectileFlags;

ThrowableRelicWeapon::ThrowableRelicWeapon(RE::TESBoundObject* a_object)
    : RelicWeapon(a_object)
{
    abilities.set(RelicAbility::kThrowable);
    if (!Initialize()) {
        spdlog::error("can't initialize throwable weapon");
    }
}

RE::BGSProjectile* ThrowableRelicWeapon::CreateBaseProjectile(const char* a_editorID, const char* a_name)
{
    auto projectileFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSProjectile>();
    if (!projectileFactory) {
        spdlog::error("projectileFactory can't find");
        return nullptr;
    }
    auto projectileBase = projectileFactory->Create();
    if (!projectileBase) {
        spdlog::error("projectileBase can't construct");
        return nullptr;
    }
    auto& formType = projectileBase->formType;
//    formType.reset(RE::FormType::ProjectileBeam);
//        formType.set(RE::FormType::ProjectileArrow);
    auto& pFlags = projectileBase->data.flags;
    pFlags.reset(pbFlag::kNone);
    pFlags.reset(pbFlag::kExplosion);
    pFlags.set(pbFlag::kCanPickUp);
    pFlags.set(pbFlag::kPassSMTransparent);
    pFlags.set(pbFlag::kDisableCombatAimCorrection);
    auto& types = projectileBase->data.types;
//    types.reset(pbType::kBeam);
    types.reset(pbType::kMissile);
    types.set(pbType::kArrow);

    const RE::BSFixedString projectileModelPath = "meshes\\WeaponThrowing\\projectiles\\NoSpin.nif";
    RE::TESBoundObject::BOUND_DATA boundData;
    boundData.boundMax = {0u, 0u, 0u};
    boundData.boundMin = {0u, 0u, 0u};
    projectileBase->SetFormEditorID(a_editorID);
    projectileBase->SetFullName(a_name);
    projectileBase->boundData = boundData;
    projectileBase->model = projectileModelPath;
    projectileBase->data.gravity = 1.f;
    projectileBase->data.speed = 1200.f;
    projectileBase->data.range = 60000.f;
    projectileBase->data.light = ThrowableWeaponLight;
    projectileBase->data.muzzleFlashLight = nullptr;
    projectileBase->data.tracerChance = 0.f;
    projectileBase->data.explosionProximity = 0.f;
    projectileBase->data.explosionTimer = 0.f;
    projectileBase->data.explosionType = nullptr;
    projectileBase->data.activeSoundLoop = nullptr;
    projectileBase->data.muzzleFlashDuration = 0.f;
    projectileBase->data.fadeOutTime = 0.f;
    projectileBase->data.force = 10.f;
    projectileBase->data.countdownSound = nullptr;
    projectileBase->data.deactivateSound = nullptr;
    projectileBase->data.defaultWeaponSource = nullptr;
    projectileBase->data.coneSpread = 0.f;
    projectileBase->data.collisionRadius = 10.f;
    projectileBase->data.lifetime = 0.f;
    projectileBase->data.relaunchInterval = 0.25f;
    projectileBase->data.decalData = nullptr;
    projectileBase->data.collisionLayer = nullptr;
    projectileBase->soundLevel = RE::SOUND_LEVEL::kNormal;
    return projectileBase;
}
RE::TESAmmo* ThrowableRelicWeapon::CreateBaseAmmo(RE::BGSProjectile* a_baseProjectile, const char* a_editorID, const char* a_name)
{
    auto ammoFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESAmmo>();
    if (!ammoFactory) {
        spdlog::error("ammoFactory can't find");
        return nullptr;
    }
    auto ammoBase = ammoFactory->Create();
    if (!ammoBase) {
        spdlog::error("ammoBase can't construct");
        return nullptr;
    }
    RE::TESBoundObject::BOUND_DATA boundData;
    boundData.boundMax = {0u, 0u, 0u};
    boundData.boundMin = {0u, 0u, 0u};
    ammoBase->SetFormEditorID(a_editorID);
    ammoBase->SetFullName(a_name);
    ammoBase->boundData = boundData;
    auto& rtData = ammoBase->GetRuntimeData();
    rtData.data.projectile = a_baseProjectile;
    rtData.data.flags.reset(RE::AMMO_DATA::Flag::kNone);
    rtData.data.flags.set(RE::AMMO_DATA::Flag::kNonPlayable);
    rtData.data.flags.set(RE::AMMO_DATA::Flag::kNonBolt);
    return ammoBase;
}

RE::NiColorA ThrowableRelicWeapon::TrailData::GetColorByIndex(const uint32_t a_index)
{
    switch ((TrailColor)a_index) {
    case TrailColor::kWhite:
        return WHITE;
    case TrailColor::kIceBlue:
        return ICEBLUE;
    case TrailColor::kSkyBlue:
        return SKYBLUE;
    case TrailColor::kBlue:
        return BLUE;
    case TrailColor::kYellow:
        return YELLOW;
    case TrailColor::kGold:
        return GOLD;
    case TrailColor::kSilver:
        return SILVER;
    default:
        return WHITE;
    }
}

bool ThrowableRelicWeapon::Initialize()
{
    ThrowableWeaponDummyProjectile = CreateBaseProjectile("DummyProjectile", "Dummy Projectile");
    ThrowableWeaponDummyAmmo = CreateBaseAmmo(ThrowableWeaponDummyProjectile, "DummyAmmo", "Dummy Ammo");
    return ThrowableWeaponDummyAmmo != nullptr;
}

RE::Actor* ThrowableRelicWeapon::GetThrowerActor() {return GetThrower() && GetThrower()->GetParent() ? GetThrower()->GetParent()->GetActor() : nullptr;}

RE::NiTransform ThrowableRelicWeapon::GetWorldTransform()
{
    if (replacedProjectileModel) {
        transformW = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(replacedProjectileModel.get());
        return transformW;
    } else if (auto droppedModel = currentReference ? currentReference->GetCurrent3D() : nullptr) {
        transformW = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(droppedModel);
        return transformW;
    }
    return {};
}
RE::NiTransform ThrowableRelicWeapon::GetLocalTransform()
{
    RE::NiTransform ret;
    if (replacedProjectileModel) {
        transformL = replacedProjectileModel->local;
        ret = transformL;
    } else ret = transformPL;
    return ret;
}

void ThrowableRelicWeapon::UpdateRotation(const float a_delta, const float a_livingTime) noexcept
{
    if (weaponParentNode) {
        auto& localRotation = weaponParentNode->local.rotate;
        if (!_rotationBlended && a_livingTime < std::max(Config::RotationBlendDuration, a_delta)) {
            const float t = std::clamp(a_livingTime / Config::RotationBlendDuration, 0.f, 1.f);
            const auto& startLocalRotationC = startLocalRotation;
            localRotation = Algebra::InterpolateRotation(startLocalRotationC, targetLocalRotation, t);
            if (rotationType != RotationType::kNone) {
                const float angleZ = -Config::ThrowRotationSpeed * a_livingTime;
                localRotation = localRotation * RE::NiMatrix3(0.f, 0.f, angleZ);
            }
        } else {
            _rotationBlended = true;
            if (rotationType != RotationType::kNone) {
                localRotation = localRotation * RE::NiMatrix3(0.f, 0.f, -Config::ThrowRotationSpeed * a_delta);
            }
        }
    }
}
void ThrowableRelicWeapon::UpdateTranslation(const float a_delta, const float a_livingTime) noexcept
{
    if (!_translationBlended && weaponParentNode) {
        auto& localTranslation = weaponParentNode->local.translate;
        if (!_translationBlended && a_livingTime < std::max(Config::RotationBlendDuration, a_delta)) {
            const float t = std::clamp(a_livingTime / Config::RotationBlendDuration, 0.f, 1.f);
            const auto& startLocalTranslationC = startLocalTranslation;
            localTranslation = Algebra::BlendVectors(startLocalTranslationC, targetLocalTranslation, t);

            if (replacedProjectileModel) {
                auto& weaponLocalTranslation = replacedProjectileModel->local.translate;
                auto rotationOriginOffset = targetLocalTranslation.y / Config::HitRotationZcos;
                weaponLocalTranslation += RE::NiPoint3(0.f, rotationOriginOffset * 0.1669f, 0.f) * a_delta / Config::RotationBlendDuration;
            }
        } else {
            _translationBlended = true;
        //    if (rotationType != RotationType::kNone) {
        //        localTranslation = localRotation * RE::NiMatrix3(0.f, 0.f, -Config::ThrowRotationSpeed * a_delta);
        //    }
        }
    }
}
void ThrowableRelicWeapon::FadeTrail()
{
    if (trailRemoveUpdate.IsTimeToUpdate()) {
        if (replacedProjectileModel) {
            if (projectile && projState == ProjectileState::kHavok) {
        //        auto& rtData = projectile->GetProjectileRuntimeData();
                auto velocity = (replacedProjectileModel->world.translate - replacedProjectileModel->previousWorld.translate) / *g_deltaTime;
                auto speed = velocity.Length();//rtData.linearVelocity.Length();
                spdlog::debug("projectile trail fading... current speed: {}", speed);
                if (speed != 0.f && speed < 669.f) {
                    RemoveTrail();
                    trailRemoveUpdate.Done();
                }
            } else {
                RemoveTrail();
                trailRemoveUpdate.Done();
            }
        }
    }
}
void ThrowableRelicWeapon::RemoveTrail()
{
    if (replacedProjectileModel && !isPenetrating) {
        if (auto trailParentBone = trailData.trailParentNode.get(); trailParentBone) {
            replacedProjectileModel->DetachChild(trailParentBone);
            _trailInitiated = false;
            OnTrailDelete();
        }
        spdlog::debug("projectile trail deleted");
    }
}

bool ThrowableRelicWeapon::Throw(Thrower* a_thrower, const RotationType a_rotationType, std::optional<ProjectileRot> a_pRot, std::optional<RE::NiPoint3> a_origin)
{
    bool result = false;
    isPenetrating = false;

    thrower = a_thrower;
    throwerParent = thrower ? thrower->GetParent() : nullptr;
    if (!thrower || !throwerParent || !throwerParent->IsValid()) {
        spdlog::debug("thrower is invalid");
        return result;
    }

    auto throwerActor = GetThrowerActor();
    if (!throwerActor) {
        spdlog::debug("thrower actor is invalid");
        return result;
    }

    auto rHandBone = throwerParent->GetRHandBone();
    auto weaponBone = throwerParent->GetWeaponBone();
    auto animObjectRBone = throwerParent->GetAnimObjectRBone();
    if (!rHandBone ||
        !weaponBone ||
        !animObjectRBone ||
        !weap ||
        !ThrowableWeaponDummyAmmo ||
        !ThrowableWeaponDummyProjectile)
    {
        spdlog::debug("thrower bones are invalid");
        return result;
    }
    rotationType = a_rotationType;

    auto weaponDamage = static_cast<float>(weap->attackDamage);
    if (throwerActor->IsPlayerRef()) {
        const auto difficulty = RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty;
        weaponDamage *= PlayerAttackDamageMultByDifficulty(difficulty);
    }

    const auto& handTransform = rHandBone->world;
    const auto& handPeviousTransform = rHandBone->previousWorld;
    const auto handVelocity = (handTransform.translate - handPeviousTransform.translate) / *g_deltaTime;    // g_deltaTimeRealTime?
    auto origin = handTransform.translate;

    if (a_pRot.has_value())
        projectileRotation = a_pRot.value();
    else
        projectileRotation = RE::Projectile::ProjectileRot(throwerActor->GetAimAngle(), throwerActor->GetAimHeading());

    auto throwDir = Algebra::PitchYawToVector(projectileRotation);
    throwDir.Unitize();
    origin += throwDir * handVelocity.Length() * *g_deltaTime;
    if (a_origin.has_value()) origin = a_origin.value();
//    RE::Projectile::LaunchData lData(ThrowableWeaponDummyProjectile, throwerActor, origin, projectileRotation);
    RE::Projectile::LaunchData lData(throwerActor, origin, projectileRotation, ThrowableWeaponDummyAmmo, weap);
    lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(throwerActor, false);
    if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(throwerActor) > 0.f)
        lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(throwerActor);

    const float impulse = thrower->GetImpulsePower(weap->weight);    // [mN.s]

    RE::NiPoint3 throwerVelocity; throwerActor->GetLinearVelocity(throwerVelocity);
    RE::NiPoint3 throwVelocity = throwerVelocity + throwDir * impulse / weap->weight;

//    ThrowableWeaponDummyProjectile->model = projectileModelPath;
//    ThrowableWeaponDummyProjectile->data.collisionRadius = 10.f;
//    ThrowableWeaponDummyProjectile->data.light = light;
    ThrowableWeaponDummyProjectile->data.speed = throwVelocity.Length();
    ThrowableWeaponDummyProjectile->data.force = impulse / 1000.f;   // [N.s]

    targetLocalRotation = RotationAngle(a_rotationType);
    targetLocalTranslation = TranslationOffset(a_rotationType, ThrowableWeaponDummyProjectile->data.collisionRadius, 0.8f);

    if (!PreThrow()) return false;

    RE::ProjectileHandle pHandle;
    if (auto pHandlePtr = RE::Projectile::Launch(&pHandle, lData); pHandlePtr && pHandlePtr->get().get()) {
        projectileHandle = *pHandlePtr;
        launchTime = AsyncUtil::GameTime::GetEngineTime();
        projectile = pHandlePtr->get().get();
        auto& rtData = projectile->GetProjectileRuntimeData();
    //    spdlog::info("throw speed: {} force: {} weaponDamage: {} difficulty: {}", ThrowableWeaponDummyProjectile->data.speed, impulse / 1000.f, weaponDamage, difficulty);
        rtData.weaponDamage = weaponDamage * thrower->GetChargeMultiplier();
        rtData.weaponDamage *= thrower->IsThrowing(ThrowType::kPowerThrow) ? 1.5f : 1.f;

        _rotationBlended = false;
        _transformInitiated = false;
        _collisionInitiated = false;

        if (!weaponModelSterilizedCopy && !replacedProjectileModel) {
            _modelInitiated = false;
            auto copyWeaponModel = weaponBone->Clone();
            auto copyWeaponModelNode = copyWeaponModel ? copyWeaponModel->AsNode() : nullptr;
            weaponModelCopy.reset(copyWeaponModelNode);
            if (weaponModelCopy) {
                weaponModelCopy->local = RE::NiTransform();
                weaponModelCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
            }
            auto copyWeaponModelSterilizedObj = weaponBone ? weaponBone->Clone() : nullptr;
            auto copyWeaponModelSterilized = copyWeaponModelSterilizedObj ? copyWeaponModelSterilizedObj->AsNode() : static_cast<RE::NiAVObject*>(copyWeaponModelSterilizedObj);
            if (copyWeaponModelSterilized) {
                copyWeaponModelSterilized->RemoveExtraData("BSX");
                copyWeaponModelSterilized->RemoveExtraData("BSXFlags");
                if (copyWeaponModelSterilized->GetCollisionObject())
                    copyWeaponModelSterilized->GetCollisionObject()->flags.reset(RE::bhkCollisionObject::Flag::kActive);
                copyWeaponModelSterilized->collisionObject.reset();
                weaponModelSterilizedCopy.reset(copyWeaponModelSterilized->AsNode());
                if (weaponModelSterilizedCopy) {
                    weaponModelSterilizedCopy->local = RE::NiTransform();
                    weaponModelSterilizedCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
                }
            }
        } else if (replacedProjectileModel) {
            weaponModelSterilizedCopy = std::move(replacedProjectileModel);
            _modelInitiated = false;
        }

        if (GetThrowState() != ThrowState::kThrowable) return true;

        SetThrowState(ThrowState::kThrown);
        if (isCountless) return true;
    //    RE::ObjectRefHandle handle;
    //    droppedWeaponKeep = RE::TESObjectREFR::CreateReference(handle, RE::FormType::Container, false);
    //    droppedWeaponKeep.get().get()->SetObjectReference(WeaponThrowing::ThrowableWeaponContainer);
//        WeaponThrowing::ThrowableWeaponContainer->SetModel(weap->GetModel());
        auto assets = Assets::GetSingleton();
        if (auto containerRef = throwerActor->PlaceObjectAtMe(assets->ThrowableWeaponContainer, false).get(); containerRef) {
            droppedWeaponKeep = containerRef->CreateRefHandle();
            droppedWeaponKeep.get()->SetTemporary();
            if (!droppedWeaponKeep.get()->IsDisabled())
                droppedWeaponKeep.get()->Disable();

            RE::ExtraUniqueID* xUID = nullptr;
            RE::ExtraDataList* xDataList = nullptr;
            auto inventory = throwerActor->GetInventory();
            for (auto& [obj, entry] : inventory) {
                if (obj == weap) {
                    if (auto eData = entry.second.get()) {
                        auto xLists = eData->extraLists;
                        xDataList = xLists && !xLists->empty() ? xLists->front() : nullptr;
                        xUID = xDataList ? xDataList->GetByType<RE::ExtraUniqueID>() : nullptr;
                    //    auto xHotkey = xDataList ? xDataList->GetByType<RE::ExtraHotkey>() : nullptr;
                    //    if (xHotkey && favoriteData && favoriteData != xHotkey) {
                    //        favoriteData = RE::ExtraHotkey::Create<RE::ExtraHotkey>();
                    //        favoriteData->hotkey = xHotkey->hotkey;
                    //        spdlog::debug("hotkey copied again");
                    //    } else if (!favoriteData) {
                    //        favoriteData = RE::ExtraHotkey::Create<RE::ExtraHotkey>();
                    //        favoriteData->hotkey = xHotkey->hotkey;
                    //        spdlog::debug("hotkey copied");
                    //    }
                    }
                }
            }

            ObjectUtil::Actor::UnEquipItem(throwerActor, false, false, false, true, true, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, throwerActor);

        //    currentReference = throwerActor->RemoveItem(weap, 1, RE::ITEM_REMOVE_REASON::kDropping, nullptr, nullptr).get().get();
        //    currentReference->Disable();
            currentReference = throwerActor->DropObject(weap, xDataList, 1, nullptr, nullptr).get().get();
            currentReference->Disable();

        //    currentReference->extraList.RemoveByType(RE::ExtraDataType::kHotkey);

            throwerParent->SetRightHandRelic(nullptr);
            isEquipped = false;
            SetWielder(nullptr);
            result = true;
        } else {
            spdlog::error("can't found container 0x1D13C from Skyrim.esm to store the weapon!");
        }
    } else {
        spdlog::error("can't create the projectile for the weapon! try with other weapons, then reinstall the mod if you can't throw any weapon.");
    }
    if (result) PostThrow();

    return result;
}
void ThrowableRelicWeapon::OnEquip(BaseActor* a_actor)
{
    if (a_actor && a_actor->IsValid()) {
        wielder = a_actor;
        lastWielder = a_actor;
        isEquipped = true;
        if (GetThrowState() != ThrowState::kArrived) {
            replacedProjectileModel.reset();
        }
        if (projectile) {
            auto& projectileRTD = projectile->GetProjectileRuntimeData();
            projectileRTD.flags.set(pFlag::kDestroyed);
            projectileModel = nullptr;
            projectileHandle.reset();
            projectile = nullptr;
        }
        SetThrowState(ThrowState::kThrowable);
        auto weapIEDnew = ObjectUtil::Actor::GetInventoryEntryDataForBoundObject(wielder->GetActor(), weap);
        if (weapIEDnew && weapIEDnew->object) {
            weapIED = std::make_unique<RE::InventoryEntryData>(*weapIEDnew);
            if (weapIED) {
                weapIED->DeepCopy(*weapIEDnew);
            }
        }
    //    weaponModelSterilizedCopy = std::move(replacedProjectileModel);
        if (a_actor->GetActor()->IsPlayerRef()) {
        //    auto inventory = a_actor->GetActor()->GetInventory();
        //    for (auto& [obj, entry] : inventory) {
        //        if (obj == weap) {
        //            if (auto eData = entry.second.get()) {
        //                auto xLists = eData->extraLists;
        //                auto xData = xLists->front();
        //                auto xHotkey = xData ? xData->GetByType<RE::ExtraHotkey>() : nullptr;
        //                if (favoriteData && favoriteData != xHotkey) {
        //                } else if (!favoriteData) {
        //                } else {
        //            //        favoriteData->hotkey.set(RE::ExtraHotkey::Hotkey::kSlot4);
        //                }
        //            }
        //        }
        //    }
        }
    } else {
        wielder = nullptr;
        isEquipped = false;
        SetThrowState(ThrowState::kNone);
    }
}
/*
bool ThrowableRelicWeapon::OnHit(RE::hkpAllCdPointCollector *a_AllCdPointCollector)
{
    const auto projBase = ThrowableWeaponDummyProjectile;
    if (projBase && projectile) {}
    return true;
}
*/
void ThrowableRelicWeapon::PostImpact(RE::Projectile::ImpactData *a_impactData, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable)
{
    RemoveTrail();
//    InitiateMapMarker();

    auto missileProjectile = projectile ? projectile->As<RE::ArrowProjectile>() : nullptr;
    if (!missileProjectile) return;
    if (a_target->GetFormType() == RE::FormType::ActorCharacter) {
        a_impactData->impactResult = RE::ImpactResult::kBounce;
        missileProjectile->GetMissileRuntimeData().impactResult = RE::ImpactResult::kBounce;
    //    if (impactType == ImpactType::kSharp) {
    //        spdlog::info("penetration depth: {}", a_collidable->allowedPenetrationDepth);
    //        if (auto victim = a_target->As<RE::Actor>(); victim) {
    //            const bool isEssential = victim->GetActorRuntimeData().boolFlags.all(RE::Actor::BOOL_FLAGS::kEssential);
    //            const bool isProtected = victim->GetActorRuntimeData().boolFlags.all(RE::Actor::BOOL_FLAGS::kProtected);
    //            if (!isEssential && !isProtected) {
    //                if (auto victimAVO = victim->AsActorValueOwner(); victimAVO) {
    //                    const float health = victimAVO->GetActorValue(RE::ActorValue::kHealth);
    //                    const float damage = missileProjectile->GetProjectileRuntimeData().weaponDamage / PlayerAttackDamageMultByDifficulty(0u);
    //                    if (health < damage) {
    //                        PickUp(victim, false);
    //                        Remove();
    //                        a_impactData->impactResult = RE::ImpactResult::kStick;
    //                        missileProjectile->GetMissileRuntimeData().impactResult = RE::ImpactResult::kStick;
    //                    }
    //                }
    //            }
    //        }
    //    }
    } else {
        if (impactType == ImpactType::kBlunt) {
            a_impactData->impactResult = RE::ImpactResult::kBounce;
            missileProjectile->GetMissileRuntimeData().impactResult = RE::ImpactResult::kBounce;
        }
    }
}
bool ThrowableRelicWeapon::GetKillOnCollision(RE::ArrowProjectile* a_this, const bool a_result)
{
    spdlog::debug("is penetrating?: {} default: {}", isPenetrating, a_result);
    return a_result;//!isPenetrating;
}
void ThrowableRelicWeapon::OnMenuOpenCloseEvent(const bool a_opening)
{
    if (a_opening) {
        GetSoundManager().PauseAllLoopingSounds();
    } else {
        GetSoundManager().ContinueAllLoopingSounds();
    }
}

bool ThrowableRelicWeapon::InitiateTransform() noexcept
{
    if (!_transformInitiated) {
        if (weaponParentNode && throwerParent->GetWeaponBone()) {
            startLocalTranslation = weaponParentNode->local.translate;
            if (weaponParentNode->parent) {
                startLocalRotation = weaponParentNode->parent->world.rotate.Transpose() * throwerParent->GetWeaponBone()->world.rotate;
            } else {
                startLocalRotation = throwerParent->GetWeaponBone()->world.rotate;
            }
            weaponParentNode->local.rotate = startLocalRotation;
            _transformInitiated = true;
        }
    } return _transformInitiated;
}
void ThrowableRelicWeapon::InitiateModel() noexcept
{
    if (!_modelInitiated) {
        if (projectileModel && projectile && projectile->Get3D() && weaponModelCopy && projectileModel == projectile->Get3D()) {
            auto weaponParentBone = projectileModel->GetChildren()[0];//->GetObjectByName(projectileAttachNodeName);
            weaponParentNode = weaponParentBone ? weaponParentBone->AsNode() : nullptr;

            replacedProjectileModel = std::move(weaponModelSterilizedCopy);

            if (weaponParentNode && replacedProjectileModel) {
                weaponParentNode->local = RE::NiTransform();
                weaponParentNode->AttachChild(replacedProjectileModel.get(), false);
                _modelInitiated = true;
                spdlog::debug("projectileModel changed!");
            } else spdlog::warn("animated node or weapon model null");
        } else spdlog::warn("projectile or projectile->Get3D2() null");
    }
}
bool ThrowableRelicWeapon::InitiateTrail() noexcept
{
    if (!_trailInitiated) {
        auto bone = replacedProjectileModel.get();
        if (bone) {
            const bool isCharged = false;//IsCharged(true);
            const float intensity = isCharged ? 3.f : 2.f;
            const auto meshOverride = isCharged ? Config::TrailModelPathFrost : Config::TrailModelPathDef;
            trailData = TrailData(meshOverride, intensity);

            if (Config::DrawTrails && (APIs::precision || APIs::Request())) {
                trailData.transformOverride.additionalRotation = RE::NiMatrix3(0.f, 0.f, -NI_HALF_PI);
                trailData.transformOverride.scale = bone->worldBound.radius * 0.01f;
                auto node = RE::NiNode::Create(0);
                trailData.trailParentNode.reset(node);
                if (node) {
                    node->name = "trailParentNode";
                    bone->AttachChild(node, false);
                    APIs::precision->AddTrailEffect(
                        node, 
                        throwerParent->GetActor()->GetParentCell(), 
                        trailData.trailOverride, 
                        trailData.transformOverride);
                    if (isCharged) {
                        trailData.trailOverride.meshOverride = Config::TrailModelPathDef;
                        APIs::precision->AddTrailEffect(
                            node, 
                            throwerParent->GetActor()->GetParentCell(), 
                            trailData.trailOverride, 
                            trailData.transformOverride);
                    }
                    _trailInitiated = true;
                }
            }
        }
    } return _trailInitiated;
}
void ThrowableRelicWeapon::Update()
{
    if (placeWeaponUpdate.IsTimeToUpdate()) {
        currentReference->Update3DPosition(true);
        currentReference->SetMotionType(RE::hkpMotion::MotionType::kKeyframed);
        if (auto droppedWeaponModel = currentReference->GetCurrent3D()) {
            const auto& desiredParentWorldRotate = replacedProjectileModel->world.rotate;
            const auto& parentWorldRotate = droppedWeaponModel->parent->world.rotate;
            droppedWeaponModel->local.rotate = desiredParentWorldRotate * parentWorldRotate.Transpose();
            currentReference->data.location = replacedProjectileModel->world.translate;
            RE::NiUpdateData updateData;
            updateData.flags.set(RE::NiUpdateData::Flag::kNone);
            updateData.time = 0.f;
            droppedWeaponModel->Update(updateData);
            placeWeaponUpdate.Done();
        }
    }
    if (trailRemoveUpdate.IsTimeToUpdate()) {
        RemoveTrail();
        trailRemoveUpdate.Done();
    }
}
void ThrowableRelicWeapon::UpdateProjectile(RE::Projectile* a_projectile)
{
    auto projectileNode = a_projectile->Get3D2();
    if (!projectileNode) {
    //    spdlog::warn("projectile's 3d not loaded");
        return;
    }
    if (projectileModel != projectileNode) {
        projectileModel = projectileNode->AsNode();
        if (GetThrowState() == ThrowState::kThrown) {
            soundData.PlayThrowingLoopSounds(projectileNode);
        }
        launchTime = AsyncUtil::GameTime::GetEngineTime();
        livingTime = 0.f;
    }
    InitiateModel();

    auto& runtimeData = a_projectile->GetProjectileRuntimeData();
    const auto& projectileLivingTime = runtimeData.livingTime;
    auto& pos   = a_projectile->data.location;
    auto& angle = a_projectile->data.angle;
    auto& vel = runtimeData.linearVelocity;
    auto dir = vel; dir.Unitize();
    position = pos;
    angles = angle;
    velocity = vel;
    direction = dir;
    if (projectileLivingTime > 0.3f && GetThrowState() == ThrowState::kThrown) SetThrowState(ThrowState::kCanArrive);

    if (projectileLivingTime > *g_deltaTime * 2.f && !InitiateTrail()) {}
    if (!InitiateTransform()) {}

   //  this is required instead of using *g_deltaTime because the game calls the GetLinearVelocityArrow function two times per frame
    const float deltaTime = GetLivingTime() - livingTime;

    static constexpr float drag = 0.00018f;
    const float speed = velocity.Length();
    vel -= direction * deltaTime * (drag * speed * speed);

    if (auto projBase = a_projectile->GetProjectileBase()) {
        if (livingTime < Config::NoGravityDurationLeviathan) {
            projBase->data.gravity = 0.f;
        } else if (projBase->data.gravity == gravity) {
            //  do nothing
        } else {
            projBase->data.gravity += deltaTime * gravity;
            if (projBase->data.gravity == gravity) {
                projBase->data.gravity = gravity;
            }
        }
    }

    UpdateRotation(deltaTime, livingTime);
    UpdateTranslation(deltaTime, livingTime);
    livingTime = GetLivingTime();
}
