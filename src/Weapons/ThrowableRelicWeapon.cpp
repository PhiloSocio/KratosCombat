#include "ThrowableRelicWeapon.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Thrower.h"
#include "Assets.h"

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
        transformW = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(replacedProjectileModel.get(), true);
        if (transformW == RE::NiTransform()) {
            transformW = replacedProjectileModel->world;
            transformW.translate /= 70.f;
        }
        return transformW;
    } else if (auto droppedModel = currentReference ? currentReference->GetCurrent3D() : nullptr) {
        transformW = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(droppedModel, true);
        if (transformW == RE::NiTransform()) {
            transformW = droppedModel->world;
            transformW.translate /= 70.f;
        }
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

bool ThrowableRelicWeapon::InitiateTransform() noexcept
{
    if (!_transformInitiated) {
        if (weaponParentNode && weaponParentNode->parent && throwerParent && throwerParent->GetWeaponBone()) {
            RE::NiUpdateData updateData;
            updateData.flags.set(RE::NiUpdateData::Flag::kNone);
            updateData.time = 0.f;
            weaponParentNode->Update(updateData);
            startLocalTranslation = weaponParentNode->local.translate;
            startLocalRotation = weaponParentNode->parent->world.rotate.Transpose() * throwerParent->GetWeaponBone()->world.rotate;

            weaponParentNode->local.rotate = startLocalRotation;
            weaponParentNode->Update(updateData);
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
                if (replacedProjectileModel->parent) {
                    replacedProjectileModel->parent->DetachChild(replacedProjectileModel.get());
                    replacedProjectileModel->parent = nullptr;
                }

                RE::NiUpdateData updateData;
                updateData.flags.set(RE::NiUpdateData::Flag::kNone);
                updateData.time = 0.f;
                replacedProjectileModel->Update(updateData);

                replacedProjectileModel->local = RE::NiTransform();
                weaponParentNode->local = RE::NiTransform();
                weaponParentNode->AttachChild(replacedProjectileModel.get(), false);
                ObjectUtil::Node::Collision::ReplaceRigidBody(projectileModel, ObjectUtil::Node::Collision::GetRigidBody(replacedProjectileModel.get()));
            //    ObjectUtil::Node::Collision::ReplaceCollisionObject(weaponParentNode, replacedProjectileModel->collisionObject);

            //    ObjectUtil::Node::Collision::RemoveCollisionObject(weaponParentNode);

            //    projectileModel->collisionObject = replacedProjectileModel->collisionObject;
            //    spdlog::debug("projectileModel: {}, child1: {}, childofchild1 {}", projectileModel->name.c_str(), projectileModel->children[0]->name.c_str(), projectileModel->children[0]->AsNode()->children[0]->name.c_str());
            //    auto collObj = projectileModel->collisionObject->AsBhkNiCollisionObject();
            //    auto bsxFlags = projectileModel->GetExtraData<RE::BSXFlags>("BSX");
            //    if (collObj && (bsxFlags->value & 1 << 6) == 0)
            //        bsxFlags->value |= 16;

                weaponParentNode->Update(updateData);

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
                        projectile->GetParentCell(), 
                        trailData.trailOverride, 
                        trailData.transformOverride);
                    if (isCharged) {
                        trailData.trailOverride.meshOverride = Config::TrailModelPathDef;
                        APIs::precision->AddTrailEffect(
                            node, 
                            projectile->GetParentCell(), 
                            trailData.trailOverride, 
                            trailData.transformOverride);
                    }
                    _trailInitiated = true;
                }
            }
        }
    } return _trailInitiated;
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
    if (weaponParentNode) {
        auto& localTranslation = weaponParentNode->local.translate;
        if (!_translationBlended && a_livingTime < std::max(Config::RotationBlendDuration * 2.f, a_delta)) {
            const float t = std::clamp(a_livingTime / Config::RotationBlendDuration * 2.f, 0.f, 1.f);
            const auto& startLocalTranslationC = startLocalTranslation;
            localTranslation = Algebra::BlendVectors(startLocalTranslationC, targetLocalTranslation, t);

            if (replacedProjectileModel) {
                auto& weaponLocalTranslation = replacedProjectileModel->local.translate;
                auto rotationOriginOffset = targetLocalTranslation.y / Config::HitRotationZcos;
                weaponLocalTranslation += RE::NiPoint3(0.f, rotationOriginOffset * 0.1669f, 0.f) * a_delta * 0.5f / Config::RotationBlendDuration;
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
            OnTrailDelete();
            replacedProjectileModel->DetachChild(trailParentBone);
            _trailInitiated = false;
            if (IsEquipped()) {
                if (replacedProjectileModel->parent) {
                    replacedProjectileModel->parent->DetachChild(replacedProjectileModel.get());
                    replacedProjectileModel->parent = nullptr;
                }
                weaponParentNode = nullptr;
                replacedProjectileModel.reset();
            }
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

    auto& rHandBone = throwerParent->GetRHandBone();
    auto& weaponBone = throwerParent->GetWeaponBone();
    auto& animObjectRBone = throwerParent->GetAnimObjectRBone();
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

    auto weaponDamage = (float)weap->attackDamage;
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
    targetLocalTranslation = TranslationOffset(a_rotationType, length + 10.f, Config::HitRotationZcos);
    spdlog::debug("length: {}", length);

    if (!PreThrow()) return false;

    soundData.PlayThrowingSounds(reinterpret_cast<RE::NiAVObject**>(&rHandBone));

    RE::ProjectileHandle pHandle;
    if (auto pHandlePtr = RE::Projectile::Launch(&pHandle, lData); pHandlePtr && pHandlePtr->get().get()) {
        projectileHandle = *pHandlePtr;
        launchTime = AsyncUtil::GameTime::GetEngineTime();
        projectile = pHandlePtr->get().get();
        projState = ProjectileState::kLaunched;

        auto& rtData = projectile->GetProjectileRuntimeData();
        rtData.weaponDamage = weaponDamage * thrower->GetChargeMultiplier();
        rtData.weaponDamage *= thrower->IsThrowing(ThrowType::kPowerThrow) ? 1.5f : 1.f;
        throwableDamage = rtData.weaponDamage;

        _rotationBlended = false;
        _translationBlended = false;
        _transformInitiated = false;
        _collisionInitiated = false;

        if (!weaponModelSterilizedCopy && !replacedProjectileModel) {
            _modelInitiated = false;
        //    RE::NiObjectPtr weaponCopy;
        //    weaponBone->CreateDeepCopy(weaponCopy);
        //    if (auto weaponCopyNode = weaponCopy ? weaponCopy->AsNode() : nullptr) {
        //        weaponModelCopy = RE::NiPointer<RE::NiNode>(weaponCopyNode);
        //    }
            auto copyWeaponModel = weaponBone->Clone();
            auto copyWeaponModelNode = copyWeaponModel ? copyWeaponModel->AsNode() : nullptr;
            weaponModelCopy.reset(copyWeaponModelNode);
            if (weaponModelCopy) {
                weaponModelCopy->local = RE::NiTransform();
                weaponModelCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
            }
            auto copyWeaponModelSterilizedObj = weaponBone->Clone();
            auto copyWeaponModelSterilized = copyWeaponModelSterilizedObj ? copyWeaponModelSterilizedObj->AsNode() : static_cast<RE::NiAVObject*>(copyWeaponModelSterilizedObj);
            if (copyWeaponModelSterilized) {
            //    ObjectUtil::Node::Collision::RemoveCollisionObject(copyWeaponModelSterilized);
                weaponModelSterilizedCopy.reset(copyWeaponModelSterilized->AsNode());
                if (weaponModelSterilizedCopy) {
                    weaponModelSterilizedCopy->local = RE::NiTransform();
                    weaponModelSterilizedCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
                }
            }
        } else if (replacedProjectileModel) {
            weaponModelSterilizedCopy = std::move(replacedProjectileModel);
            if (weaponModelSterilizedCopy->parent) {
                weaponModelSterilizedCopy->parent->DetachChild(weaponModelSterilizedCopy.get());
                weaponModelSterilizedCopy->parent = nullptr;
            }
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
    if (result) {
        PostThrow();
        spdlog::info("{} throwed by {}, raw damage is: {}", weap->GetFullName(), throwerActor->GetName(), throwableDamage);
    }

    return result;
}
void ThrowableRelicWeapon::OnEquip(BaseActor* a_actor)
{
    RelicWeapon::OnEquip(a_actor);
    if (a_actor && a_actor->IsValid()) {
        if (GetThrowState() != ThrowState::kArrived) {
            replacedProjectileModel.reset();
        }
        if (projectile) {
            auto& projectileRTD = projectile->GetProjectileRuntimeData();
            projectileRTD.flags.set(pFlag::kDestroyed);
            projectileHandle.reset();
            projectileModel = nullptr;
            projectile = nullptr;
        }
        SetThrowState(ThrowState::kThrowable);
        projState = ProjectileState::kNone;
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
        if (GetThrowState() == ThrowState::kThrowable)
            SetThrowState(ThrowState::kNone);
    }
}
void ThrowableRelicWeapon::PostImpact(RE::Projectile::ImpactData *a_impactData, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable)
{
    RemoveTrail();
//    InitiateMapMarker();

    if (!a_target) return;

    auto missileProjectile = projectile ? projectile->As<RE::ArrowProjectile>() : nullptr;
    if (!missileProjectile) return;

    if (a_target->GetFormType() == RE::FormType::ActorCharacter) {
        a_impactData->impactResult = RE::ImpactResult::kBounce;
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
        }
        if (a_impactData->impactResult < RE::ImpactResult::kStick) {
            a_impactData->impactResult = RE::ImpactResult::kBounce;
        }
    }
    switch (a_impactData->impactResult) {
    case RE::ImpactResult::kBounce:  projState = ProjectileState::kHavok;   break;
    case RE::ImpactResult::kStick:   projState = ProjectileState::kStucked; break;
    case RE::ImpactResult::kDestroy: projState = ProjectileState::kNone;    break;
    case RE::ImpactResult::kImpale:  projState = ProjectileState::kNone;    break;
    case RE::ImpactResult::kNone:    projState = ProjectileState::kNone;    break;
    default: ProjectileState::kNone; break;
    }
}
bool ThrowableRelicWeapon::GetKillOnCollision(RE::ArrowProjectile* a_this, const bool a_result)
{
    spdlog::debug("is penetrating?: {} default: {}", isPenetrating, a_result);
    auto& flags = projectile->GetProjectileRuntimeData().flags;
    flags = isPenetrating ? flags.reset(pFlag::kDestroyAfterHit) : flags.set(pFlag::kDestroyAfterHit);
    return isPenetrating;
}
void ThrowableRelicWeapon::OnMenuOpenCloseEvent(const bool a_opening)
{
    if (a_opening) {
        GetSoundManager().PauseAllLoopingSounds();
    } else {
        GetSoundManager().ContinueAllLoopingSounds();
    }
}

void ThrowableRelicWeapon::Update()
{
    soundData.Update();
    if (placeWeaponUpdate.IsTimeToUpdate()) {
        currentReference->Update3DPosition(true);
        currentReference->SetMotionType(RE::hkpMotion::MotionType::kKeyframed);
        if (auto droppedWeaponModel = currentReference->GetCurrent3D()) {
            const auto& desiredParentWorldRotate = replacedProjectileModel->world.rotate;
            const auto& parentWorldRotate = droppedWeaponModel->parent->world.rotate;
            droppedWeaponModel->local.rotate = parentWorldRotate.Transpose() * desiredParentWorldRotate;
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
void ThrowableRelicWeapon::UpdateProjectile(RE::Projectile* a_projectile, float a_delta)
{
    auto projectileNode = a_projectile->Get3D2();
    if (!projectileNode) {
    //    spdlog::warn("projectile's 3d not loaded");
        return;
    }
    if (projectileModel != projectileNode) {
        projectileModel = projectileNode->AsFadeNode();
        if (GetThrowState() == ThrowState::kThrown) {
            soundData.PlayThrowingLoopSounds(reinterpret_cast<RE::NiAVObject**>(&projectileModel));
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
    livingTime = GetLivingTime();

    if (projState == ProjectileState::kTransition) return;

    if (projectileLivingTime > 0.3f && GetThrowState() == ThrowState::kThrown) SetThrowState(ThrowState::kCanArrive);
    if (projectileLivingTime > a_delta * 2.f && !InitiateTrail()) {}

    if (GetThrowState() == ThrowState::kArriving) return;

    if (!InitiateTransform()) {}

    static constexpr float drag = 0.0003f;
    const float speed = velocity.Length();
    vel -= direction * a_delta * (drag * speed * speed);

    if (auto projBase = a_projectile->GetProjectileBase()) {
        if (livingTime < Config::NoGravityDurationLeviathan) {
            projBase->data.gravity = 0.f;
        } else if (projBase->data.gravity == gravity) {
            //  do nothing
        } else {
            projBase->data.gravity += a_delta * gravity;
            if (projBase->data.gravity == gravity) {
                projBase->data.gravity = gravity;
            }
        }
    }

    UpdateRotation(a_delta, livingTime);
    UpdateTranslation(a_delta, livingTime);
}
