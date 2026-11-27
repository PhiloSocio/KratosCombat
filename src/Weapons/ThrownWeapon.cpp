#include "ThrownWeapon.h"
#include "ThrowableRelicWeapon.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Thrower.h"
#include "Assets.h"
#include "util.h"

using namespace MathUtil;
using namespace GameSettingUtil;

ThrownWeapon::ThrownWeapon(ThrowableRelicWeapon& a_weapon)
    : weapon(a_weapon),
      weap(a_weapon.weap),
      currentReference(a_weapon.currentReference),
      weapIED(a_weapon.weapIED)
{
    ThrowableWeaponDummyProjectile = weapon.ThrowableWeaponDummyProjectile;
    ThrowableWeaponDummyAmmo = weapon.ThrowableWeaponDummyAmmo;
    if (ThrowableWeaponDummyProjectile)
        currentGravity.store(ThrowableWeaponDummyProjectile->data.gravity);
    throwState = ThrowState::kThrowable;
}

const RE::NiColorA& ThrownWeapon::TrailData::GetColorByIndex(const uint32_t a_index) const noexcept
{
    switch ((TrailColor)a_index) {
    case TrailColor::kWhite:   return WHITE;
    case TrailColor::kIceBlue: return ICEBLUE;
    case TrailColor::kSkyBlue: return SKYBLUE;
    case TrailColor::kBlue:    return BLUE;
    case TrailColor::kYellow:  return YELLOW;
    case TrailColor::kGold:    return GOLD;
    case TrailColor::kSilver:  return SILVER;
    default:                   return WHITE;
    }
}
const std::string& ThrownWeapon::TrailData::GetTrailMeshByEnchantment(const RE::EnchantmentItem* a_ench) const noexcept
{
    const auto enchEffect  = (a_ench && !a_ench->effects.empty()) ? a_ench->effects[0] : nullptr;
    const auto baseEffect  = enchEffect ? enchEffect->baseEffect : nullptr;
    const auto keywords    = baseEffect ? baseEffect->keywords : nullptr;
    const auto keyword     = (keywords && *keywords) ? keywords[0] : nullptr;
    const auto keywordName = keyword ? static_cast<std::string_view>(keyword->formEditorID) : std::string_view{};
    if (keywordName.empty())                                   return Config::TrailModelPathDef;
    if (keywordName.find("Frost"sv) != std::string_view::npos) return Config::TrailModelPathFrost;
    if (keywordName.find("Shock"sv) != std::string_view::npos) return Config::TrailModelPathShock;
    if (keywordName.find("Fire"sv)  != std::string_view::npos) return Config::TrailModelPathFire;
                                                               return Config::TrailModelPathMagic;
}
ThrownWeapon::TrailData::TrailData(const std::string a_meshDirectory, const float a_intensity, const uint32_t a_colorIndex)
{
    trailOverride = TrailOverride(
        Config::TrailLifetimeMult * a_intensity,
        GetColorByIndex(a_colorIndex),
        Config::TrailColorScaleMult * a_intensity,
        a_meshDirectory
    );
}

bool ThrownWeapon::IsEquipped() const noexcept
{
    return weapon.IsEquipped();
}
BaseActor* ThrownWeapon::GetCarrier() const noexcept
{
    return weapon.GetCarrier();
}
SoundManager& ThrownWeapon::GetSoundManager() const noexcept { return weapon.GetSoundManager(); }
RE::Actor* ThrownWeapon::GetThrowerActor() const
{
    return GetThrower() && GetThrower()->GetParent() ? GetThrower()->GetParent()->GetActor() : nullptr;
}
RE::NiTransform ThrownWeapon::GetWorldTransform()
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
RE::NiTransform ThrownWeapon::GetLocalTransform()
{
    RE::NiTransform ret;
    if (replacedProjectileModel) {
        transformL = replacedProjectileModel->local;
        ret = transformL;
    } else ret = transformPL;
    return ret;
}

bool ThrownWeapon::InitiateTransform() noexcept
{
    if (!_transformInitiated) {
        if (weaponParentNode && weaponParentNode->parent && projectileModel) {
            RE::NiUpdateData updateData;
            updateData.flags.set(RE::NiUpdateData::Flag::kNone);
            updateData.time = 0.f;
            if (!_hitTransformApplied) {
                Algebra::SetRotationMatrix(projectileModel->local.rotate, -direction.x, direction.y, direction.z);
                projectileModel->Update(updateData);
            }
            weaponParentNode->Update(updateData);
            startLocalTranslation = weaponParentNode->local.translate;
            startLocalRotation = weaponParentNode->parent->world.rotate.Transpose() * startWorldRotation;

            weaponParentNode->local.rotate = startLocalRotation;
            weaponParentNode->Update(updateData);
            _transformInitiated = true;
        }
    } return _transformInitiated;
}
void ThrownWeapon::InitiateModel() noexcept
{
    if (!_modelInitiated) {
        if (projectileModel && projectile && projectile->Get3D() && weaponModelCopy && projectileModel.get() == projectile->Get3D()) {
            auto weaponParentBone = projectileModel->GetChildren()[0];//->GetObjectByName(projectileAttachNodeName);
            weaponParentNode.reset(weaponParentBone ? weaponParentBone->AsNode() : nullptr);

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
                ObjectUtil::Node::Collision::ReplaceRigidBody(projectileModel.get(), ObjectUtil::Node::Collision::GetRigidBody(replacedProjectileModel.get()));
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
bool ThrownWeapon::InitiateTrail() noexcept
{
    if (!_trailInitiated) {
        auto bone = replacedProjectileModel.get();
        if (bone) {
            const bool isCharged = weapon.IsCharged(*this);
            const float intensity = isCharged ? 1.69f : 1.f;
            const auto meshOverride = isCharged ? trailData.GetTrailMeshByEnchantment(ench) : Config::TrailModelPathDef;

            trailData.trailOverride.meshOverride       = meshOverride;
            trailData.trailOverride.baseColorScaleMult = Config::TrailColorScaleMult * intensity;
            trailData.trailOverride.lifetimeMult       = Config::TrailLifetimeMult * intensity;

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
void ThrownWeapon::UpdateRotation(const float a_delta, const float a_livingTime, RE::NiPoint3& a_angles) noexcept
{
    if (weaponParentNode && !_hitTransformApplied) {
        auto& localRotation = weaponParentNode->local.rotate;
        const float blendDuration = std::max(Config::RotationBlendDuration, 1e-6f);
        if (!_rotationBlended && a_livingTime < std::max(blendDuration, a_delta)) {
            const float linearT = std::clamp((a_livingTime + a_delta) / blendDuration, 0.f, 1.f);
            const float t = linearT * linearT * (3.f - 2.f * linearT);
            localRotation = Algebra::InterpolateRotation(const_cast<const RE::NiMatrix3&>(startLocalRotation), targetLocalRotation, t);
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
        a_angles.x = asin(direction.z);
        a_angles.z = atan2(direction.x, direction.y);
        if (a_angles.z < 0.f) {
            a_angles.z += PI;
        }
        if (direction.x < 0.f) {
            a_angles.z += PI;
        }
        Algebra::SetRotationMatrix(projectileModel->local.rotate, -direction.x, direction.y, direction.z);
        spdlog::info("rotation is {}, {}, {}", angles.x, angles.y, angles.z);
    }
}
void ThrownWeapon::UpdateTranslation(const float a_delta, const float a_livingTime) noexcept
{
    if (weaponParentNode) {
        auto& localTranslation = weaponParentNode->local.translate;
        if (!_translationBlended && a_livingTime < std::max(Config::RotationBlendDuration * 2.f, a_delta)) {
            const float t = std::clamp(a_livingTime / Config::RotationBlendDuration / 2.f, 0.f, 1.f);
            localTranslation = Algebra::BlendVectors(const_cast<const RE::NiPoint3&>(startLocalTranslation), targetLocalTranslation, t);

            if (replacedProjectileModel) {
                auto& weaponLocalTranslation = replacedProjectileModel->local.translate;
                auto rotationOriginOffset = targetLocalTranslation.y / Config::HitRotationZcos;
                weaponLocalTranslation += RE::NiPoint3(0.f, rotationOriginOffset * 0.1669f, 0.f) * a_delta * 0.5f / Config::RotationBlendDuration;
            }

            if (_hitTransformApplied) {
                localTranslation = targetLocalTranslation;
                _translationBlended = true;
            }
        } else {
            _translationBlended = true;
        //    if (rotationType != RotationType::kNone) {
        //        localTranslation = localRotation * RE::NiMatrix3(0.f, 0.f, -Config::ThrowRotationSpeed * a_delta);
        //    }
        }
    }
}
void ThrownWeapon::FadeTrail()
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
void ThrownWeapon::RemoveTrail()
{
    OnTrailDelete();
    if (replacedProjectileModel) {
        auto trailParentBone = trailData.trailParentNode.get();
        if (trailParentBone && trailParentBone->parent == replacedProjectileModel.get()) {
            replacedProjectileModel->DetachChild(trailParentBone);
            _trailInitiated = false;
            if (IsEquipped()) {
                if (replacedProjectileModel->parent) {
                    replacedProjectileModel->parent->DetachChild(replacedProjectileModel.get());
                    replacedProjectileModel->parent = nullptr;
                }
                weaponParentNode.reset();
                replacedProjectileModel.reset();
            }
        }
        trailData.trailParentNode.reset();
        spdlog::debug("{} projectile trail deleted", weap->GetFullName());
    }
}

bool ThrownWeapon::Throw(Thrower* a_thrower, const RotationType a_rotationType, std::optional<ProjectileRot> a_pRot, std::optional<RE::NiPoint3> a_origin, const ThrowType a_throwType, const float a_chargeDuration)
{
    bool result = false;
    throwingChargeDuration = a_chargeDuration;
    isPenetrating = false;
    isDestroyed.store(false);
    destructionHandled = false;
    gravityFrozen.store(false);
    isAuxiliary = false;
    isThrown = false;

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
    throwingType = a_throwType;
    startWorldRotation = weaponBone->world.rotate;

    auto weaponDamage = (float)weap->attackDamage;
    if (throwerActor->IsPlayerRef()) {
        const auto difficulty = RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty;
        weaponDamage *= PlayerAttackDamageMultByDifficulty(difficulty);
    }

    const auto& handTransform = rHandBone->world;
    const auto& handPeviousTransform = rHandBone->previousWorld;
    const auto handVelocity = (handTransform.translate - handPeviousTransform.translate) / *g_deltaTime;    // g_deltaTimeRealTime?
    auto origin = handTransform.translate;

    if (a_pRot.has_value()) {
        projectileRotation = a_pRot.value();
    } else if (a_throwType == ThrowType::kMelee) {
        throwerActor->Unk_A0(weaponBone, projectileRotation.x, projectileRotation.z, origin);
    } else {
        projectileRotation = RE::Projectile::ProjectileRot(throwerActor->GetAimAngle(), throwerActor->GetAimHeading());
    }
    auto throwDir = Algebra::PitchYawToVector(projectileRotation);
    throwDir.Unitize();

    if (a_throwType == ThrowType::kMelee) {
        origin -= throwDir * length * 0.5f;
    } else {
        origin += throwDir * handVelocity.Length() * *g_deltaTime;
    }

    if (a_origin.has_value()) origin = a_origin.value();

//    RE::Projectile::LaunchData lData(ThrowableWeaponDummyProjectile, throwerActor, origin, projectileRotation);
    RE::Projectile::LaunchData lData(throwerActor, origin, projectileRotation, ThrowableWeaponDummyAmmo, weap);
    lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(throwerActor, false);

    ench = nullptr;
    if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(throwerActor) > 0.f)
        lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(throwerActor);

    ench = lData.enchantItem;

    const float impulse = thrower->GetImpulsePower(weap->weight);    // [mN.s]

    RE::NiPoint3 throwerVelocity; throwerActor->GetLinearVelocity(throwerVelocity);
    RE::NiPoint3 throwVelocity = throwerVelocity + throwDir * impulse / weap->weight;

    length = weaponBone->worldBound.radius;
    targetLocalRotation = RotationAngle(a_rotationType);
    targetLocalTranslation = TranslationOffset(a_rotationType, length, Config::HitRotationZcos);

    float collisionRadius = a_rotationType == RotationType::kNone ? 6.9f : length * 0.36f;

    auto& projBaseData = ThrowableWeaponDummyProjectile->data;
    projBaseData.collisionRadius = collisionRadius;
//    projBaseData.light = light;
    projBaseData.speed = throwVelocity.Length();
    projBaseData.force = impulse / 1000.f;   // [N.s]
    projBaseData.range = 69696.9f;
    if (a_throwType == ThrowType::kMelee)
        projBaseData.range = length > 69.f ? length * 2.1f : 137.f + 6.9f;

    if (!weapon.PreThrow(*this, lData)) return false;

    GetSoundManager().PlayThrowingSounds(reinterpret_cast<RE::NiAVObject**>(&rHandBone));

    RE::ProjectileHandle pHandle;
    if (auto pHandlePtr = RE::Projectile::Launch(&pHandle, lData); pHandlePtr && pHandlePtr->get().get()) {
        projectileHandle = *pHandlePtr;
        launchTime = AsyncUtil::GameTime::GetEngineTime();
        projectile = pHandlePtr->get().get();
        projectile->inGameFormFlags.set(RE::TESForm::InGameFormFlag::kForcedPersistent);
        projState = ProjectileState::kLaunched;
        currentGravity.store(projBaseData.gravity);
        gravityFrozen.store(false);

        auto& rtData = projectile->GetProjectileRuntimeData();
        rtData.weaponDamage = weaponDamage * thrower->GetChargeMultiplier();
        rtData.weaponDamage *= thrower->IsThrowing(ThrowType::kPowerThrow) ? 1.5f : 1.f;
        throwableDamage = rtData.weaponDamage;

        _rotationBlended = false;
        _translationBlended = false;
        _transformInitiated = false;
        _hitTransformApplied = false;
        _collisionInitiated = false;
        _trailInitiated = false;

        if (weapon.isCountless || (!weaponModelSterilizedCopy && !replacedProjectileModel)) {
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

        if (weapon.isCountless) {

            spdlog::info("{} throwed by {}, raw damage is: {}", weap->GetFullName(), throwerActor->GetName(), throwableDamage);
            return true;
        }
        if (GetThrowState() != ThrowState::kThrowable) return true;

        SetThrowState(ThrowState::kThrown);
    //    RE::ObjectRefHandle handle;
    //    weapon.droppedWeaponKeep = RE::TESObjectREFR::CreateReference(handle, RE::FormType::Container, false);
    //    weapon.droppedWeaponKeep.get().get()->SetObjectReference(WeaponThrowing::ThrowableWeaponContainer);
//        WeaponThrowing::ThrowableWeaponContainer->SetModel(weap->GetModel());
        auto assets = Assets::GetSingleton();
        if (!weapon.droppedWeaponKeep.get().get()) {
            if (auto containerRef = throwerActor->PlaceObjectAtMe(assets->ThrowableWeaponContainer, true).get(); containerRef) {
                weapon.droppedWeaponKeep = containerRef->CreateRefHandle();
                weapon.droppedWeaponKeep.get()->inGameFormFlags.set(RE::TESForm::InGameFormFlag::kForcedPersistent);
                if (!weapon.droppedWeaponKeep.get()->IsDisabled())
                    weapon.droppedWeaponKeep.get()->Disable();
            }
        }
        if (weapon.droppedWeaponKeep.get().get()) {
            ObjectUtil::Actor::UnEquipItem(throwerActor, false, false, false, true, true, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, throwerActor);

            RE::ExtraDataList* xDataList = ExtraDataUtil::FindExtraListWithUniqueID(throwerActor, weap);
            if (!xDataList)
                spdlog::warn("Throw: {} has no unique ID list, its identity may change after pickup!", weap->GetFullName());

            currentReference = weapon.droppedWeaponKeep.get().get();
            throwerActor->RemoveItem(weap, 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, weapon.droppedWeaponKeep.get().get());
        //    currentReference = throwerActor->DropObject(weap, xDataList, 1, nullptr, nullptr).get().get();
        //    currentReference->Disable();

        //    currentReference->extraList.RemoveByType(RE::ExtraDataType::kHotkey);

            throwerParent->SetRightHandRelic(nullptr);
            weapon.isEquipped = false;
            weapon.SetWielder(nullptr);
            result = true;
        } else {
            spdlog::error("can't found container 0x1D13C from Skyrim.esm to store the weapon!");
        }
    } else {
        spdlog::error("can't create the projectile for the weapon! try with other weapons, then reinstall the mod if you can't throw any weapon.");
    }
    if (result) {

        spdlog::info("{} throwed by {}, raw damage is: {}", weap->GetFullName(), throwerActor->GetName(), throwableDamage);
    }

    return result;
}

void ThrownWeapon::UpdateProjectile(RE::Projectile* a_this, float a_delta)
{
    auto projectileNode = a_this->Get3D2();
    if (!projectileNode) {
        spdlog::warn("projectile {}'s 3d not loaded", a_this->GetName());
        return;
    }
    if (projectileModel.get() != projectileNode) {
        projectileModel.reset(projectileNode->AsFadeNode());
        if (GetThrowState() == ThrowState::kThrown) {
            GetSoundManager().PlayThrowingLoopSounds(reinterpret_cast<RE::NiAVObject**>(&projectileModel));
        }
        launchTime = AsyncUtil::GameTime::GetEngineTime();
        livingTime = 0.f;
    }
    InitiateModel();

    auto& runtimeData = a_this->GetProjectileRuntimeData();
    const auto& projectileLivingTime = runtimeData.livingTime;
    auto& pos   = a_this->data.location;
    auto& angle = a_this->data.angle;
    auto& vel = runtimeData.linearVelocity;
    auto dir = vel; dir.Unitize();

    livingTime = GetLivingTime();
    position   = pos;
    velocity   = vel;

    if (_hitTransformApplied) { //  to prevent impact orientation application
        angle = angles;
    } else {
        direction = dir;
        angles = angle;
    }


    if (projState == ProjectileState::kTransition) return;

    if (projectileLivingTime > 0.3f && GetThrowState() == ThrowState::kThrown) SetThrowState(ThrowState::kCanArrive);
    if (projectileLivingTime > a_delta * 2.f && !InitiateTrail()) {}

    if (GetThrowState() == ThrowState::kArriving) return;

    if (!InitiateTransform()) {}

    const float speed = velocity.Length();
    vel -= direction * a_delta * (aeroDragCoeff * speed * speed);

    if (!gravityFrozen.load(std::memory_order_relaxed)) {
        auto current = currentGravity.load(std::memory_order_relaxed);
        if (livingTime < Config::NoGravityDurationLeviathan) {
            currentGravity.store(0.f, std::memory_order_relaxed);
        } else if (current < gravity) {
            currentGravity.store(std::min(gravity, current + a_delta * gravity), std::memory_order_relaxed);
        }
    }

    UpdateRotation(a_delta, livingTime, angle);
    UpdateTranslation(a_delta, livingTime);
}
bool ThrownWeapon::OnHit(RE::ArrowProjectile* a_projectile, RE::hkpAllCdPointCollector* a_collector)
{
    return weapon.OnHit(*this, a_projectile, a_collector);
}
void ThrownWeapon::PostImpact(RE::ArrowProjectile* a_projectile, RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable)
{
    weapon.PostImpact(*this, a_projectile, a_impactData, a_target, a_targetLoc, a_velocity, a_collidable);
}
void ThrownWeapon::ResetProjectile()
{
    if (projectile) {
        auto current = projectile;
        weapon.UnhookProjectile(current);
        if (projectileHandle.get().get() == current)
            current->Kill();
        projectileModel.reset();
        projectileHandle.reset();
        projectile = nullptr;
        isDestroyed.store(true, std::memory_order_release);
        destructionHandled = true;
    }
}
void ThrownWeapon::Update(float a_delta)
{
    GetSoundManager().Update();
    if (isDestroyed.load(std::memory_order_acquire)) {
        if (!destructionHandled) {
            projectileModel.reset();
            projectileHandle.reset();
            projectile = nullptr;
            destructionHandled = true;
        }
        FadeTrail();
        return;
    }
    FadeTrail();
}
