#include "ThrowableRelicWeapon.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Thrower.h"
#include "Assets.h"
#include "hook.h"
#include "util.h"

using namespace MathUtil;
using namespace GameSettingUtil;

using pbType = RE::BGSProjectileData::Type;
using pbFlag = RE::BGSProjectileData::BGSProjectileFlags;

ThrowableRelicWeapon::~ThrowableRelicWeapon()
{
    UnhookProjectiles();
}
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

const RE::NiColorA& ThrowableRelicWeapon::TrailData::GetColorByIndex(const uint32_t a_index) const noexcept
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
const std::string& ThrowableRelicWeapon::TrailData::GetTrailMeshByEnchantment(const RE::EnchantmentItem* a_ench) const noexcept
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
bool ThrowableRelicWeapon::InitiateTrail() noexcept
{
    if (!_trailInitiated) {
        auto bone = replacedProjectileModel.get();
        if (bone) {
            const bool isCharged = IsCharged(true);
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
void ThrowableRelicWeapon::UpdateRotation(const float a_delta, const float a_livingTime, RE::NiPoint3& a_angles) noexcept
{
    if (weaponParentNode && !_hitTransformApplied) {
        auto& localRotation = weaponParentNode->local.rotate;
        if (!_rotationBlended && a_livingTime < std::max(Config::RotationBlendDuration, a_delta)) {
            const float t = std::clamp(a_livingTime / Config::RotationBlendDuration, 0.f, 1.f);
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
void ThrowableRelicWeapon::UpdateTranslation(const float a_delta, const float a_livingTime) noexcept
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

bool ThrowableRelicWeapon::Throw(Thrower* a_thrower, const RotationType a_rotationType, std::optional<ProjectileRot> a_pRot, std::optional<RE::NiPoint3> a_origin, const ThrowType a_throwType)
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
    throwingType = a_throwType;

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

    if (!PreThrow(lData)) return false;

    soundData.PlayThrowingSounds(reinterpret_cast<RE::NiAVObject**>(&rHandBone));

    RE::ProjectileHandle pHandle;
    if (auto pHandlePtr = RE::Projectile::Launch(&pHandle, lData); pHandlePtr && pHandlePtr->get().get()) {
        projectileHandle = *pHandlePtr;
        launchTime = AsyncUtil::GameTime::GetEngineTime();
        projectile = pHandlePtr->get().get();
        projectile->inGameFormFlags.set(RE::TESForm::InGameFormFlag::kForcedPersistent);
        projState = ProjectileState::kLaunched;
        HookProjectile(projectile);

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

        if (isCountless || (!weaponModelSterilizedCopy && !replacedProjectileModel)) {
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

        if (isCountless) {
            PostThrow();
            spdlog::info("{} throwed by {}, raw damage is: {}", weap->GetFullName(), throwerActor->GetName(), throwableDamage);
            return true;
        }
        if (GetThrowState() != ThrowState::kThrowable) return true;

        SetThrowState(ThrowState::kThrown);
    //    RE::ObjectRefHandle handle;
    //    droppedWeaponKeep = RE::TESObjectREFR::CreateReference(handle, RE::FormType::Container, false);
    //    droppedWeaponKeep.get().get()->SetObjectReference(WeaponThrowing::ThrowableWeaponContainer);
//        WeaponThrowing::ThrowableWeaponContainer->SetModel(weap->GetModel());
        auto assets = Assets::GetSingleton();
        if (!droppedWeaponKeep.get().get()) {
            if (auto containerRef = throwerActor->PlaceObjectAtMe(assets->ThrowableWeaponContainer, true).get(); containerRef) {
                droppedWeaponKeep = containerRef->CreateRefHandle();
                droppedWeaponKeep.get()->inGameFormFlags.set(RE::TESForm::InGameFormFlag::kForcedPersistent);
                if (!droppedWeaponKeep.get()->IsDisabled())
                    droppedWeaponKeep.get()->Disable();
            }
        }
        if (droppedWeaponKeep.get().get()) {
            ObjectUtil::Actor::UnEquipItem(throwerActor, false, false, false, true, true, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, throwerActor);

            RE::ExtraDataList* xDataList = ExtraDataUtil::FindExtraListWithUniqueID(throwerActor, weap);
            if (!xDataList)
                spdlog::warn("Throw: {} has no unique ID list, its identity may change after pickup!", weap->GetFullName());

            currentReference = droppedWeaponKeep.get().get();
            throwerActor->RemoveItem(weap, 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, droppedWeaponKeep.get().get());
        //    currentReference = throwerActor->DropObject(weap, xDataList, 1, nullptr, nullptr).get().get();
        //    currentReference->Disable();

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
bool ThrowableRelicWeapon::HookProjectile(RE::Projectile* a_this)
{
    if (!a_this) return false;

    auto ctx = ProjectileInstanceHook::GetOrCreate(a_this);
    if (ctx && ctx->userData && ctx->userData != this) {
        static_cast<ThrowableRelicWeapon*>(ctx->userData)->projectileHooks.erase(a_this);
        ProjectileInstanceHook::Destroy(a_this);
        ctx = ProjectileInstanceHook::Create(a_this);
    }
    if (!ctx) return false;

    ctx->userData = this;
    ctx->Hook<kUpdateArrow>(&ThrowableRelicWeapon::UpdateArrow)
        ->Hook<kOnArrowKill>(&ThrowableRelicWeapon::OnArrowKill)
        ->Hook<kGetCollisionArrow>(&ThrowableRelicWeapon::GetCollisionArrow)
        ->Hook<kGetArrowImpactData>(&ThrowableRelicWeapon::GetArrowImpactData)
        ->Apply();
    projectileHooks.insert_or_assign(a_this, ctx);
    return true;
}
void ThrowableRelicWeapon::UnhookProjectile(RE::Projectile* a_this)
{
    if (!a_this) return;

    auto ctx = ProjectileInstanceHook::From(a_this);
    if (!ctx || ctx->userData != this) return;

    projectileHooks.erase(a_this);
    ProjectileInstanceHook::Destroy(a_this);
}
void ThrowableRelicWeapon::UnhookProjectiles()
{
    for (auto& [proj, ctx] : projectileHooks)
        ProjectileInstanceHook::Destroy(ctx);
    projectileHooks.clear();
}
void ThrowableRelicWeapon::ResetProjectile()
{
    if (projectile) {
        UnhookProjectile(projectile);
        if (projectileHandle.get().get() == projectile)
            projectile->Kill();
        projectileModel.reset();
        projectileHandle.reset();
        projectile = nullptr;
    }
}
void ThrowableRelicWeapon::UpdateArrow(RE::ArrowProjectile* a_this, float a_delta)
{
    auto ctx = ProjectileInstanceHook::From(a_this);
    if (!ctx) return;

    auto _UpdateArrow = ctx->Original<decltype(&ThrowableRelicWeapon::UpdateArrow), kUpdateArrow>();
    auto relic = static_cast<ThrowableRelicWeapon*>(ctx->userData);
    _UpdateArrow(a_this, a_delta);
    if (relic && relic->projectile == a_this) {
        if (relic->projState < ProjectileState::kStucked)
            relic->UpdateProjectile(a_this, a_delta);
    }
}
void ThrowableRelicWeapon::OnArrowKill(RE::ArrowProjectile* a_this)
{
    auto ctx = ProjectileInstanceHook::From(a_this);
    if (!ctx) return;

    auto _OnArrowKill = ctx->Original<decltype(&ThrowableRelicWeapon::OnArrowKill), kOnArrowKill>();
    auto relic = static_cast<ThrowableRelicWeapon*>(ctx->userData);
    if (relic) {
        relic->UnhookProjectile(a_this);
        if (relic->projectile == a_this) {
            relic->projectileModel.reset();
            relic->projectileHandle.reset();
            relic->projectile = nullptr;
        }
    }
    _OnArrowKill(a_this);
}
void ThrowableRelicWeapon::GetCollisionArrow(RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
    auto ctx = ProjectileInstanceHook::From(a_this);
    if (!ctx) return;

    auto _GetCollisionArrow = ctx->Original<decltype(&ThrowableRelicWeapon::GetCollisionArrow), kGetCollisionArrow>();
    auto relic = static_cast<ThrowableRelicWeapon*>(ctx->userData);
    if (relic && relic->OnHit(a_this, a_AllCdPointCollector)) return;
    AttackHook::BeforeDamage(a_this, a_AllCdPointCollector);
    _GetCollisionArrow(a_this, a_AllCdPointCollector);
}
RE::Projectile::ImpactData* ThrowableRelicWeapon::GetArrowImpactData(RE::ArrowProjectile* a_this, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable, std::uint32_t a6, std::uint32_t a7)
{
    auto ctx = ProjectileInstanceHook::From(a_this);
    if (!ctx) return nullptr;

    auto _GetArrowImpactData = ctx->Original<decltype(&ThrowableRelicWeapon::GetArrowImpactData), kGetArrowImpactData>();
    auto impactData = _GetArrowImpactData(a_this, a_target, a_targetLoc, a_velocity, a_collidable, a6, a7);
    auto relic = static_cast<ThrowableRelicWeapon*>(ctx->userData);
    if (relic)
        relic->PostImpact(a_this, impactData, a_target, a_targetLoc, a_velocity, a_collidable);
    return impactData;
}
void ThrowableRelicWeapon::OnEquip(BaseActor* a_actor)
{
    RelicWeapon::OnEquip(a_actor);
    if (a_actor && a_actor->IsValid()) {
        if (GetThrowState() != ThrowState::kArrived) {
            RemoveTrail();
            weaponParentNode.reset();
            replacedProjectileModel.reset();
        }
        projectile = projectileHandle.get().get();
        ResetProjectile();
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
void ThrowableRelicWeapon::PostImpact(RE::ArrowProjectile* a_this, RE::Projectile::ImpactData *a_impactData, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable)
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
void ThrowableRelicWeapon::UpdateProjectile(RE::Projectile* a_this, float a_delta)
{
    auto projectileNode = a_this->Get3D2();
    if (!projectileNode) {
        spdlog::warn("projectile {}'s 3d not loaded", a_this->GetName());
        return;
    }
    if (projectileModel.get() != projectileNode) {
        projectileModel.reset(projectileNode->AsFadeNode());
        if (GetThrowState() == ThrowState::kThrown) {
            soundData.PlayThrowingLoopSounds(reinterpret_cast<RE::NiAVObject**>(&projectileModel));
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

    static constexpr float drag = 0.0003f;
    const float speed = velocity.Length();
    vel -= direction * a_delta * (drag * speed * speed);

    if (auto projBase = a_this->GetProjectileBase()) {
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

    UpdateRotation(a_delta, livingTime, angle);
    UpdateTranslation(a_delta, livingTime);
}
