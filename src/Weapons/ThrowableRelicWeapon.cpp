#include "ThrowableRelicWeapon.h"
#include "Assets.h"

using namespace MathUtil;
using namespace GameSettingUtil;

using pbType = RE::BGSProjectileData::Type;
using pbFlag = RE::BGSProjectileData::BGSProjectileFlags;

ThrowableRelicWeapon::ThrowableRelicWeapon(RE::TESBoundObject* a_object)
    : RelicWeapon(a_object)
{
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

RE::NiTransform ThrowableRelicWeapon::GetWorldTransform()
{
    if (runtimeData.replacedProjectileModel) {
        runtimeData.transformW = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(runtimeData.replacedProjectileModel.get());
        return runtimeData.transformW;
    } else return runtimeData.transformPW;
    return {};
}
RE::NiTransform ThrowableRelicWeapon::GetLocalTransform()
{
    RE::NiTransform ret;
    if (runtimeData.replacedProjectileModel) {
        runtimeData.transformL = runtimeData.replacedProjectileModel->local;
        ret = runtimeData.transformL;
    } else ret = runtimeData.transformPL;
    return ret;
}

void ThrowableRelicWeapon::AddTrail()
{
    if (trailUpdate.IsTimeToUpdate()) {
        trailRemoveUpdate.Done();
        RemoveTrail();
        auto bone = runtimeData.replacedProjectileModel;
        if (bone) {
            const bool isCharged = IsCharged(true);
            const float intensity = isCharged ? 3.f : 2.f;
            const auto meshOverride = isCharged ? Config::TrailModelPathFrost : Config::TrailModelPathDef;
            float length = bone->worldBound.radius;
            ObjectUtil::Capsule capsule;
            ObjectUtil::Node::GetCapsuleParams(bone->AsNode(), capsule);
            float capsuleLength = capsule.a.GetDistance(capsule.b);
            length = length > capsuleLength ? length : capsuleLength;
            float scale = fmax(length, capsule.radius) * 0.01f;
            float tipOffset = length;
            trailData = TrailData(meshOverride, intensity);

            if (Config::UsePrecisionTrails && (Config::IsPrecisionInstalled || APIs::precision || APIs::Request())) {
                trailUpdate.Done();
                trailData.transformOverride.additionalRotation = RE::NiMatrix3(0.f, 0.f, -NI_HALF_PI);
                trailData.transformOverride.scale = bone->worldBound.radius * 0.01f;
                auto node = RE::NiNode::Create(0);
                node->name = "trailParentNode";
                bone->AttachChild(node, false);
                APIs::precision->AddTrailEffect(
                    node, 
                    RE::PlayerCharacter::GetSingleton()->GetParentCell(), 
                    trailData.trailOverride, 
                    trailData.transformOverride);
                if (isCharged) {
                    trailData.trailOverride.meshOverride = Config::TrailModelPathDef;
                    APIs::precision->AddTrailEffect(
                        node, 
                        RE::PlayerCharacter::GetSingleton()->GetParentCell(), 
                        trailData.trailOverride, 
                        trailData.transformOverride);
                }
            //    APIs::precision->AddAttackCollision(RE::PlayerCharacter::GetSingleton()->GetHandle(), collisionDefinition, LastLeviProjectile);
            }
        }
    }
}
void ThrowableRelicWeapon::FadeTrail()
{
    if (trailRemoveUpdate.IsTimeToUpdate()) {
        if (runtimeData.replacedProjectileModel) {
            if (runtimeData.projectile && runtimeData.projState == ProjectileState::kHavok) {
        //        auto& rtData = runtimeData.projectile->GetProjectileRuntimeData();
                auto velocity = (runtimeData.replacedProjectileModel->world.translate - runtimeData.replacedProjectileModel->previousWorld.translate) / *g_deltaTime;
                auto speed = velocity.Length();//rtData.linearVelocity.Length();
                spdlog::debug("projectile trail fading... current speed: {}", speed);
                if (speed != 0.f && speed < 669.f) {
                    RemoveTrail();
                    trailRemoveUpdate.Done();
                }
            } else {
                RemoveTrail();
                runtimeData.replacedProjectileModel.reset();
                trailRemoveUpdate.Done();
            }
        }
    }
}
void ThrowableRelicWeapon::RemoveTrail()
{
    if (runtimeData.replacedProjectileModel) {
        auto trailParentBone = runtimeData.replacedProjectileModel->GetObjectByName("trailParentNode");
        runtimeData.replacedProjectileModel->DetachChild(trailParentBone);
    //    if (runtimeData.replacedProjectileModel->parent)
    //        runtimeData.replacedProjectileModel->parent->DetachChild(runtimeData.replacedProjectileModel.get());
        OnTrailDelete();
        spdlog::debug("projectile trail deleted");
    }
}

bool ThrowableRelicWeapon::Throw(const RotationType a_rotationType, std::optional<ProjectileRot> a_pRot, std::optional<RE::NiPoint3> a_origin)
{
    bool result = false;

    auto throwerActor = GetThrowerActor();
    if (!throwerActor) return result;

    auto thrower = GetThrower();
    if (!thrower || !thrower->IsValid()) return result;

    auto rHandBone = thrower->GetRHandBone();
    auto weaponBone = thrower->GetWeaponBone();
    auto animObjectRBone = thrower->GetAnimObjectRBone();
    if (!rHandBone ||
        !weaponBone ||
        !animObjectRBone ||
        !weap ||
        !ThrowableWeaponDummyAmmo ||
        !ThrowableWeaponDummyProjectile) {
        return result;
    }

    auto weaponDamage = static_cast<float>(weap->attackDamage);
    if (throwerActor->IsPlayerRef()) {
        const auto difficulty = RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty;
        weaponDamage *= PlayerAttackDamageMultByDifficulty(difficulty);
    }

    const auto& handTransform = rHandBone->world;
    const auto& handPeviousTransform = rHandBone->previousWorld;
    const auto handVelocity = (handTransform.translate - handPeviousTransform.translate) / *g_deltaTime;    // g_deltaTimeRealTime?
    auto origin = handTransform.translate;

    RE::ProjectileHandle pHandle;
    if (a_pRot.has_value())
        runtimeData.projectileRotation = a_pRot.value();
    else
        runtimeData.projectileRotation = RE::Projectile::ProjectileRot(throwerActor->GetAimAngle(), throwerActor->GetAimHeading());

    auto throwDir = Algebra::PitchYawToVector(runtimeData.projectileRotation);
    throwDir.Unitize();
    origin += throwDir * handVelocity.Length() * *g_deltaTime;
    if (a_origin.has_value()) origin = a_origin.value();
//    RE::Projectile::LaunchData lData(ThrowableWeaponDummyProjectile, throwerActor, origin, runtimeData.projectileRotation);
    RE::Projectile::LaunchData lData(throwerActor, origin, runtimeData.projectileRotation, ThrowableWeaponDummyAmmo, weap);
    lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(throwerActor, false);
    if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(throwerActor) > 0.f)
        lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(throwerActor);

    const float impulse = thrower->GetImpulsePower(weap->weight);    // [mN.s]

    RE::NiPoint3 throwerVelocity; throwerActor->GetLinearVelocity(throwerVelocity);
    RE::NiPoint3 throwVelocity = throwerVelocity + throwDir * impulse / weap->weight;

//    ThrowableWeaponDummyProjectile->model = runtimeData.projectileModelPath;
//    ThrowableWeaponDummyProjectile->data.collisionRadius = 10.f;
//    ThrowableWeaponDummyProjectile->data.light = runtimeData.light;
    ThrowableWeaponDummyProjectile->data.speed = throwVelocity.Length();
    ThrowableWeaponDummyProjectile->data.force = impulse / 1000.f;   // [N.s]

    if (!PreThrow()) return false;

    if (runtimeData.projectileHandle = RE::Projectile::Launch(&pHandle, lData); runtimeData.projectileHandle && runtimeData.projectileHandle->get().get()) {
        runtimeData.projectile = runtimeData.projectileHandle->get().get();
        auto& rtData = runtimeData.projectile->GetProjectileRuntimeData();
    //    spdlog::info("throw speed: {} force: {} weaponDamage: {} difficulty: {}", ThrowableWeaponDummyProjectile->data.speed, impulse / 1000.f, weaponDamage, difficulty);
        rtData.weaponDamage = weaponDamage * thrower->GetChargeMultiplier();
        rtData.weaponDamage *= thrower->IsThrowing(ThrowType::kPowerThrowing) ? 1.5f : 1.f;

        auto copyWeaponModel = weaponBone->Clone();
        auto copyWeaponModelNode = copyWeaponModel ? copyWeaponModel->AsNode() : nullptr;
        runtimeData.weaponModelCopy.reset(copyWeaponModelNode);
        if (runtimeData.weaponModelCopy) {
            runtimeData.weaponModelCopy->local = RE::NiTransform();
            runtimeData.weaponModelCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
        }
        auto copyWeaponModelSterilizedObj = weaponBone ? weaponBone->Clone() : nullptr;
        auto copyWeaponModelSterilized = copyWeaponModelSterilizedObj ? copyWeaponModelSterilizedObj->AsNode() : static_cast<RE::NiAVObject*>(copyWeaponModelSterilizedObj);
        if (copyWeaponModelSterilized) {
            copyWeaponModelSterilized->RemoveExtraData("BSX");
            copyWeaponModelSterilized->RemoveExtraData("BSXFlags");
            if (copyWeaponModelSterilized->GetCollisionObject())
                copyWeaponModelSterilized->GetCollisionObject()->flags.reset(RE::bhkCollisionObject::Flag::kActive);
            copyWeaponModelSterilized->collisionObject.reset();
            runtimeData.weaponModelSterilizedCopy.reset(copyWeaponModelSterilized->AsNode());
            if (runtimeData.weaponModelSterilizedCopy) {
                runtimeData.weaponModelSterilizedCopy->local = RE::NiTransform();
                runtimeData.weaponModelSterilizedCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
            }
        }

        if (runtimeData.isCountless) return true;
    //    RE::ObjectRefHandle handle;
    //    runtimeData.droppedWeaponKeep = RE::TESObjectREFR::CreateReference(handle, RE::FormType::Container, false);
    //    runtimeData.droppedWeaponKeep.get().get()->SetObjectReference(WeaponThrowing::ThrowableWeaponContainer);
//        WeaponThrowing::ThrowableWeaponContainer->SetModel(weap->GetModel());
        auto assets = Assets::GetSingleton();
        if (auto containerRef = throwerActor->PlaceObjectAtMe(assets->ThrowableWeaponContainer, false).get(); containerRef) {
            runtimeData.droppedWeaponKeep = containerRef->CreateRefHandle();
            runtimeData.droppedWeaponKeep.get()->SetTemporary();
            if (!runtimeData.droppedWeaponKeep.get()->IsDisabled())
                runtimeData.droppedWeaponKeep.get()->Disable();

            ObjectUtil::Actor::UnEquipItem(throwerActor, false, false, false, true, true, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, throwerActor);
            throwerActor->RemoveItem(weap, 1, RE::ITEM_REMOVE_REASON::kStoreInContainer, nullptr, GetWeaponContainer());
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
/*
bool ThrowableRelicWeapon::OnHit(RE::hkpAllCdPointCollector *a_AllCdPointCollector)
{
    const auto projBase = ThrowableWeaponDummyProjectile;
    if (projBase && runtimeData.projectile) {}
    return true;
}
*/
void ThrowableRelicWeapon::PostImpact(RE::Projectile::ImpactData *a_impactData, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable)
{
    RemoveTrail();
//    InitiateMapMarker();

    auto missileProjectile = runtimeData.projectile ? runtimeData.projectile->As<RE::ArrowProjectile>() : nullptr;
    if (!missileProjectile) return;
    if (a_target->GetFormType() == RE::FormType::ActorCharacter) {
        a_impactData->impactResult = RE::ImpactResult::kBounce;
        missileProjectile->GetMissileRuntimeData().impactResult = RE::ImpactResult::kBounce;
    //    if (runtimeData.impactType == ImpactType::kSharp) {
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
        if (runtimeData.impactType == ImpactType::kBlunt) {
            a_impactData->impactResult = RE::ImpactResult::kBounce;
            missileProjectile->GetMissileRuntimeData().impactResult = RE::ImpactResult::kBounce;
        }
    }
}
void ThrowableRelicWeapon::OnMenuOpenCloseEvent(const bool a_opening)
{
    if (a_opening) {
        GetSoundManager().PauseAllLoopingSounds();
    } else {
        GetSoundManager().ContinueAllLoopingSounds();
    }
}
void ThrowableRelicWeapon::Update() {
    if (projectileUpdate.IsTimeToUpdate()) {
        if (runtimeData.projectileModel && runtimeData.projectile && runtimeData.projectile->Get3D() && runtimeData.weaponModelCopy && runtimeData.projectileModel == runtimeData.projectile->Get3D()) {
            const RE::BSFixedString rotatingBoneName = "Cylinder02";
            auto animatedBone = runtimeData.projectileModel->GetObjectByName(rotatingBoneName);
            auto animatedNode = animatedBone ? animatedBone->AsNode() : nullptr;

            auto cloneModel = runtimeData.weaponModelCopy.get()->Clone();
            auto cloneNode = cloneModel ? cloneModel->AsNode() : nullptr;
            runtimeData.replacedProjectileModel.reset(cloneNode);

            if (animatedNode) {
                animatedNode->AttachChild(runtimeData.replacedProjectileModel.get(), false);
            //    auto oldWorld = runtimeData.transformW;
            //    oldWorld.translate *= 70.f;
            //    oldWorld.scale = runtimeData.replacedProjectileModel.get()->world.scale;
            //    runtimeData.replacedProjectileModel.get()->local = ObjectUtil::Node::GetLocalTransform(runtimeData.replacedProjectileModel.get(), oldWorld);
                projectileUpdate.Done();
                trailUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);
                spdlog::debug("levi projectileModel changed!");
            } else spdlog::warn("animated node or levinode null");
        } else spdlog::warn("projectile or projectile->Get3D2() null");
    }
    if (soundData.arrivingLoopStopUpdate.IsTimeToUpdate()) {soundData.StopArrivingLoopSounds();}
    if (soundData.throwingLoopStopUpdate.IsTimeToUpdate()) {soundData.StopThrowingLoopSounds();}
    if (Config::DrawTrails) {
        AddTrail();
        FadeTrail();
    }
//    if (runtimeData.replacedProjectileModel) {
//        auto projNiTransform = runtimeData.replacedProjectileModel->world;
//        auto projBHKTransform = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(runtimeData.replacedProjectileModel.get());
//        auto projHKPTransform = ObjectUtil::Node::GetHavokHKPRigidBodyWorldTransform(runtimeData.replacedProjectileModel.get());
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
