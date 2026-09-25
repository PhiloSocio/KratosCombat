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

bool ThrowableRelicWeapon::Initialize()
{
    ThrowableWeaponDummyProjectile = CreateBaseProjectile("DummyProjectile", "Dummy Projectile");
    ThrowableWeaponDummyAmmo = CreateBaseAmmo(ThrowableWeaponDummyProjectile, "DummyAmmo", "Dummy Ammo");
    if (ThrowableWeaponDummyAmmo) return true;
}

bool ThrowableRelicWeapon::Throw(const RotationType a_rotationType)
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
    throwableWeaponRuntimeData.projectileRotation = RE::Projectile::ProjectileRot(throwerActor->GetAimAngle(), throwerActor->GetAimHeading());
    auto throwDir = Algebra::PitchYawToVector(throwableWeaponRuntimeData.projectileRotation);
    throwDir.Unitize();
    origin += throwDir * handVelocity.Length() * *g_deltaTime;
//    RE::Projectile::LaunchData lData(ThrowableWeaponDummyProjectile, throwerActor, origin, throwableWeaponRuntimeData.projectileRotation);
    RE::Projectile::LaunchData lData(throwerActor, origin, throwableWeaponRuntimeData.projectileRotation, ThrowableWeaponDummyAmmo, weap);
    lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(throwerActor, false);
    if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(throwerActor) > 0.f)
        lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(throwerActor);

    const float impulse = thrower->GetImpulsePower(weap->weight);    // [mN.s]

    RE::NiPoint3 throwerVelocity; throwerActor->GetLinearVelocity(throwerVelocity);
    RE::NiPoint3 throwVelocity = throwerVelocity + throwDir * impulse / weap->weight;

//    ThrowableWeaponDummyProjectile->model = throwableWeaponRuntimeData.projectileModelPath;
//    ThrowableWeaponDummyProjectile->data.collisionRadius = 10.f;
//    ThrowableWeaponDummyProjectile->data.light = throwableWeaponRuntimeData.light;
    ThrowableWeaponDummyProjectile->data.speed = throwVelocity.Length();
    ThrowableWeaponDummyProjectile->data.force = impulse / 1000.f;   // [N.s]

    if (!PreThrow()) return false;

    if (throwableWeaponRuntimeData.projectileHandle = RE::Projectile::Launch(&pHandle, lData); throwableWeaponRuntimeData.projectileHandle && throwableWeaponRuntimeData.projectileHandle->get().get()) {
        throwableWeaponRuntimeData.projectile = throwableWeaponRuntimeData.projectileHandle->get().get();
        auto& rtData = throwableWeaponRuntimeData.projectile->GetProjectileRuntimeData();
    //    spdlog::info("throw speed: {} force: {} weaponDamage: {} difficulty: {}", ThrowableWeaponDummyProjectile->data.speed, impulse / 1000.f, weaponDamage, difficulty);
        rtData.weaponDamage = weaponDamage * thrower->GetChargeMultiplier();
        rtData.weaponDamage *= thrower->IsThrowing(ThrowType::kPowerThrowing) ? 1.5f : 1.f;

        auto copyWeaponModel = weaponBone->Clone();
        auto copyWeaponModelNode = copyWeaponModel ? copyWeaponModel->AsNode() : nullptr;
        throwableWeaponRuntimeData.weaponModelCopy.reset(copyWeaponModelNode);
        if (throwableWeaponRuntimeData.weaponModelCopy) {
            throwableWeaponRuntimeData.weaponModelCopy->local = RE::NiTransform();
            throwableWeaponRuntimeData.weaponModelCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
        }
        auto copyWeaponModelSterilizedObj = weaponBone ? weaponBone->Clone() : nullptr;
        auto copyWeaponModelSterilized = copyWeaponModelSterilizedObj ? copyWeaponModelSterilizedObj->AsNode() : static_cast<RE::NiAVObject*>(copyWeaponModelSterilizedObj);
        if (copyWeaponModelSterilized) {
            copyWeaponModelSterilized->RemoveExtraData("BSX");
            copyWeaponModelSterilized->RemoveExtraData("BSXFlags");
            if (copyWeaponModelSterilized->GetCollisionObject())
                copyWeaponModelSterilized->GetCollisionObject()->flags.reset(RE::bhkCollisionObject::Flag::kActive);
            copyWeaponModelSterilized->collisionObject.reset();
            throwableWeaponRuntimeData.weaponModelSterilizedCopy.reset(copyWeaponModelSterilized->AsNode());
            if (throwableWeaponRuntimeData.weaponModelSterilizedCopy) {
                throwableWeaponRuntimeData.weaponModelSterilizedCopy->local = RE::NiTransform();
                throwableWeaponRuntimeData.weaponModelSterilizedCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
            }
        }

    //    RE::ObjectRefHandle handle;
    //    throwableWeaponRuntimeData.droppedWeaponKeep = RE::TESObjectREFR::CreateReference(handle, RE::FormType::Container, false);
    //    throwableWeaponRuntimeData.droppedWeaponKeep.get().get()->SetObjectReference(WeaponThrowing::ThrowableWeaponContainer);
//        WeaponThrowing::ThrowableWeaponContainer->SetModel(weap->GetModel());
        auto assets = Assets::GetSingleton();
        if (auto containerRef = throwerActor->PlaceObjectAtMe(assets->ThrowableWeaponContainer, false).get(); containerRef) {
            throwableWeaponRuntimeData.droppedWeaponKeep = containerRef->CreateRefHandle();
            throwableWeaponRuntimeData.droppedWeaponKeep.get()->SetTemporary();
            if (!throwableWeaponRuntimeData.droppedWeaponKeep.get()->IsDisabled())
                throwableWeaponRuntimeData.droppedWeaponKeep.get()->Disable();

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

