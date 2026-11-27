#include "ThrowableRelicWeapon.h"
#include "ThrownSmartWeapon.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Thrower.h"
#include "Assets.h"
#include "hook.h"
#include "util.h"

using namespace MathUtil;
using namespace GameSettingUtil;

using pbType = RE::BGSProjectileData::Type;
using pbFlag = RE::BGSProjectileData::BGSProjectileFlags;
using ProjectileInstanceHook = HookUtil::InstanceHook<RE::Projectile, 0x100>;

ThrowableRelicWeapon::ThrowableRelicWeapon(RE::TESBoundObject* a_object)
    : RelicWeapon(a_object)
{
    abilities.set(RelicAbility::kThrowable);
    thrownWeapons.reserve(32);
    updateBuffer.reserve(32);
    retiredThrownWeapons.reserve(32);
    if (!Initialize())
        spdlog::error("can't initialize throwable weapon");
}
ThrowableRelicWeapon::~ThrowableRelicWeapon()
{
    UnhookProjectiles();
    thrownWeapons.clear();
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

    auto& pFlags = projectileBase->data.flags;
    pFlags.reset(pbFlag::kNone);
    pFlags.reset(pbFlag::kExplosion);
    pFlags.set(pbFlag::kCanPickUp);
    pFlags.set(pbFlag::kPassSMTransparent);
    pFlags.set(pbFlag::kDisableCombatAimCorrection);

    auto& types = projectileBase->data.types;
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
    if (!a_baseProjectile) return nullptr;

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
    return ThrowableWeaponDummyProjectile && ThrowableWeaponDummyAmmo;
}
std::unique_ptr<ThrownWeapon> ThrowableRelicWeapon::CreateThrownWeapon()
{
    return std::make_unique<ThrownWeapon>(*this);
}
bool ThrowableRelicWeapon::Throw(Thrower* a_thrower, const RotationType a_rotationType, std::optional<ProjectileRot> a_pRot, std::optional<RE::NiPoint3> a_origin, const ThrowType a_throwType, const float a_chargeDuration)
{
    auto runtime = CreateThrownWeapon();
    if (!runtime || !runtime->Throw(a_thrower, a_rotationType, a_pRot, a_origin, a_throwType, a_chargeDuration))
        return false;

    auto projectile = runtime->projectile;
    if (!projectile) return false;

    CleanupThrownWeapons();
    if (!RetireConflictingProjectile(projectile)) {
        spdlog::error("projectile pointer collision while registering thrown weapon");
        projectile->Kill();
        return false;
    }
    auto runtimePtr = runtime.get();
    const bool inserted = thrownWeapons.emplace(projectile, std::move(runtime)).second;
    if (!inserted) {
        spdlog::error("projectile pointer collision while registering thrown weapon");
        projectile->Kill();
        return false;
    }

    if (!HookProjectile(*runtimePtr)) {
        runtimePtr->isDestroyed.store(true, std::memory_order_release);
        projectile->Kill();
        return false;
    }

    lastThrownWeapon = runtimePtr;
    PostThrow(*runtimePtr);
    runtimePtr->currentGravity.store(runtimePtr->gravity, std::memory_order_release);
    return true;
}
bool ThrowableRelicWeapon::RegisterAuxiliaryProjectile(RE::Projectile* a_projectile, const RE::ProjectileHandle& a_handle)
{
    if (!a_projectile) return false;

    CleanupThrownWeapons();
    auto runtime = CreateThrownWeapon();
    if (!runtime) {
        a_projectile->Kill();
        return false;
    }
    runtime->projectile = a_projectile;
    runtime->projectileHandle = a_handle;
    runtime->isAuxiliary = true;
    runtime->throwState = ThrowState::kThrown;
    if (!RetireConflictingProjectile(a_projectile)) {
        spdlog::error("projectile pointer collision while registering auxiliary projectile");
        a_projectile->Kill();
        return false;
    }
    auto runtimePtr = runtime.get();
    if (!thrownWeapons.emplace(a_projectile, std::move(runtime)).second) {
        a_projectile->Kill();
        return false;
    }
    if (!HookProjectile(*runtimePtr)) {
        runtimePtr->isDestroyed.store(true, std::memory_order_release);
        a_projectile->Kill();
        return false;
    }
    return true;
}
ThrownWeapon* ThrowableRelicWeapon::GetThrownWeapon(RE::Projectile* a_projectile) const noexcept
{
    const auto it = thrownWeapons.find(a_projectile);
    return it != thrownWeapons.end() ? it->second.get() : nullptr;
}
Thrower* ThrowableRelicWeapon::GetThrower() const noexcept
{
    return lastThrownWeapon ? lastThrownWeapon->GetThrower() : nullptr;
}
RE::Actor* ThrowableRelicWeapon::GetThrowerActor() const
{
    return lastThrownWeapon ? lastThrownWeapon->GetThrowerActor() : nullptr;
}

bool ThrowableRelicWeapon::HasActiveThrownWeapon() const noexcept
{
    for (const auto& [projectile, runtime] : thrownWeapons) {
        if (runtime && !runtime->isDestroyed.load(std::memory_order_acquire) && !runtime->isAuxiliary && runtime->projectile == projectile)
            return true;
    }
    return false;
}
bool ThrowableRelicWeapon::OnHit(ThrownWeapon&, RE::ArrowProjectile*, RE::hkpAllCdPointCollector*)
{
    return false;
}
void ThrowableRelicWeapon::PostImpact(ThrownWeapon& a_weapon, RE::ArrowProjectile* a_projectile, RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3*, RE::NiPoint3*, RE::hkpCollidable*)
{
    if (!a_projectile || !a_impactData || !a_target) return;

    if (a_target->GetFormType() == RE::FormType::ActorCharacter) {
        a_impactData->impactResult = RE::ImpactResult::kBounce;
    } else {
        if (a_weapon.impactType == ImpactType::kBlunt)
            a_impactData->impactResult = RE::ImpactResult::kBounce;
        if (a_impactData->impactResult < RE::ImpactResult::kStick)
            a_impactData->impactResult = RE::ImpactResult::kBounce;
    }

    switch (a_impactData->impactResult) {
    case RE::ImpactResult::kBounce:  a_weapon.projState = ProjectileState::kHavok; break;
    case RE::ImpactResult::kStick:   a_weapon.projState = ProjectileState::kStucked; break;
    case RE::ImpactResult::kDestroy:
    case RE::ImpactResult::kImpale:
    case RE::ImpactResult::kNone:    a_weapon.projState = ProjectileState::kNone; break;
    default:                         a_weapon.projState = ProjectileState::kNone; break;
    }
}
bool ThrowableRelicWeapon::PreThrow(ThrownWeapon&, RE::Projectile::LaunchData&)
{
    return true;
}
void ThrowableRelicWeapon::PostThrow(ThrownWeapon&)
{}

bool ThrowableRelicWeapon::HookProjectile(ThrownWeapon& a_weapon)
{
    auto projectile = a_weapon.projectile;
    if (!projectile) return false;

    auto ctx = ProjectileInstanceHook::GetOrCreate(projectile);
    if (ctx && ctx->userData && ctx->userData != &a_weapon) {
        auto oldRuntime = static_cast<ThrownWeapon*>(ctx->userData);
        oldRuntime->weapon.UnhookProjectile(projectile);
        ctx = ProjectileInstanceHook::Create(projectile);
    }
    if (!ctx) return false;

    ctx->userData = &a_weapon;
    ctx->Hook<kUpdateArrow>(&ThrowableRelicWeapon::UpdateArrow)
        ->Hook<kOnArrowKill>(&ThrowableRelicWeapon::OnArrowKill)
        ->Hook<kGetCollisionArrow>(&ThrowableRelicWeapon::GetCollisionArrow)
        ->Hook<kGetArrowImpactData>(&ThrowableRelicWeapon::GetArrowImpactData)
        ->Hook<kGetGravityArrow>(&ThrowableRelicWeapon::GetGravityArrow)
        ->Apply();
    return true;
}
void ThrowableRelicWeapon::UnhookProjectile(RE::Projectile* a_projectile)
{
    if (!a_projectile) return;
    auto ctx = ProjectileInstanceHook::From(a_projectile);
    if (!ctx) return;
    auto runtime = static_cast<ThrownWeapon*>(ctx->userData);
    if (runtime && &runtime->weapon != this) return;
    ProjectileInstanceHook::Destroy(a_projectile);
}
void ThrowableRelicWeapon::UnhookProjectiles()
{
    for (auto& [projectile, runtime] : thrownWeapons) {
        auto ctx = ProjectileInstanceHook::From(projectile);
        if (ctx && runtime && ctx->userData == runtime.get())
            ProjectileInstanceHook::Destroy(ctx);
    }
}
bool ThrowableRelicWeapon::RetireConflictingProjectile(RE::Projectile* a_projectile)
{
    auto it = thrownWeapons.find(a_projectile);
    if (it == thrownWeapons.end()) return true;

    auto runtime = it->second.get();
    const auto registeredProjectile = runtime ? runtime->projectileHandle.get().get() : nullptr;
    const bool isStale = !runtime || runtime->isDestroyed.load(std::memory_order_acquire)
        || runtime->projectile != a_projectile || registeredProjectile != a_projectile || a_projectile->IsDestroyed();
    if (!isStale) return false;

    if (runtime) {
        UnhookProjectile(a_projectile);
        runtime->RemoveTrail();
        runtime->projectileModel.reset();
        runtime->projectile = nullptr;
        runtime->projectileHandle.reset();
        runtime->isDestroyed.store(true, std::memory_order_release);
        runtime->destructionHandled = true;
        if (runtime == lastThrownWeapon) lastThrownWeapon = nullptr;
        runtime->GetSoundManager().FadeAllSounds(200u);
        retiredThrownWeapons.emplace_back(updateCounter + 1u, std::move(it->second));
    } else {
        UnhookProjectile(a_projectile);
    }
    thrownWeapons.erase(it);
    return true;
}
void ThrowableRelicWeapon::CleanupThrownWeapons()
{
    for (auto it = thrownWeapons.begin(); it != thrownWeapons.end();) {
        auto runtime = it->second.get();
        const bool keepLastSmartRuntime = HasAbility(RelicAbility::kSmart) && runtime == lastThrownWeapon;
        const bool hasPendingTrail = runtime && (runtime->replacedProjectileModel || runtime->weaponParentNode || runtime->trailData.trailParentNode);
        if (runtime && runtime->isDestroyed.load(std::memory_order_acquire) && !keepLastSmartRuntime && !hasPendingTrail) {
            if (runtime->retireUpdate == 0) {
                runtime->retireUpdate = updateCounter;
                ++it;
            } else if (updateCounter > runtime->retireUpdate) {
                if (runtime == lastThrownWeapon) lastThrownWeapon = nullptr;
                it = thrownWeapons.erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    std::erase_if(retiredThrownWeapons, [this](const auto& retired) {
        return updateCounter > retired.first;
    });
}
bool ThrowableRelicWeapon::RebindProjectile(ThrownWeapon& a_weapon, RE::Projectile* a_projectile, const RE::ProjectileHandle& a_handle)
{
    if (!a_projectile) return false;
    if (a_weapon.projectile == a_projectile) return true;

    auto node = a_weapon.projectile ? thrownWeapons.extract(a_weapon.projectile) : decltype(thrownWeapons)::node_type{};
    if (node.empty()) {
        const auto it = std::find_if(thrownWeapons.begin(), thrownWeapons.end(), [&a_weapon](const auto& entry) {
            return entry.second.get() == &a_weapon;
        });
        if (it != thrownWeapons.end())
            node = thrownWeapons.extract(it);
    }
    if (node.empty()) {
        spdlog::error("thrown weapon runtime is not registered");
        a_projectile->Kill();
        return false;
    }

    if (thrownWeapons.contains(a_projectile)) {
        thrownWeapons.insert(std::move(node));
        spdlog::error("projectile pointer collision while rebinding thrown weapon");
        a_projectile->Kill();
        return false;
    }

    node.key() = a_projectile;
    a_weapon.projectile = a_projectile;
    a_weapon.projectileHandle = a_handle;
    a_weapon.isDestroyed.store(false, std::memory_order_release);
    a_weapon.destructionHandled = false;
    if (!thrownWeapons.insert(std::move(node)).inserted) {
        spdlog::error("projectile pointer collision while rebinding thrown weapon");
        a_weapon.isDestroyed.store(true, std::memory_order_release);
        a_projectile->Kill();
        return false;
    }

    if (!HookProjectile(a_weapon)) {
        a_weapon.isDestroyed.store(true, std::memory_order_release);
        a_projectile->Kill();
        return false;
    }
    return true;
}
void ThrowableRelicWeapon::ResetProjectile()
{
    for (auto& [projectile, runtime] : thrownWeapons)
        if (runtime) runtime->ResetProjectile();
}
void ThrowableRelicWeapon::UpdateArrow(RE::ArrowProjectile* a_this, float a_delta)
{
    auto ctx = ProjectileInstanceHook::From(a_this);
    if (!ctx) return;

    auto original = ctx->Original<decltype(&ThrowableRelicWeapon::UpdateArrow), kUpdateArrow>();
    auto runtime = static_cast<ThrownWeapon*>(ctx->userData);
    original(a_this, a_delta);
    if (runtime && runtime->projectile == a_this && !runtime->isDestroyed.load(std::memory_order_acquire) && !runtime->isAuxiliary && runtime->projState < ProjectileState::kStucked)
        runtime->UpdateProjectile(a_this, a_delta);
}
void ThrowableRelicWeapon::OnArrowKill(RE::ArrowProjectile* a_this)
{
    auto ctx = ProjectileInstanceHook::From(a_this);
    if (!ctx) return;

    auto original = ctx->Original<decltype(&ThrowableRelicWeapon::OnArrowKill), kOnArrowKill>();
    auto runtime = static_cast<ThrownWeapon*>(ctx->userData);
    if (runtime) {
        runtime->weapon.UnhookProjectile(a_this);
        runtime->isDestroyed.store(true, std::memory_order_release);
    }
    original(a_this);
}
void ThrowableRelicWeapon::GetCollisionArrow(RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_collector)
{
    auto ctx = ProjectileInstanceHook::From(a_this);
    if (!ctx) return;

    auto original = ctx->Original<decltype(&ThrowableRelicWeapon::GetCollisionArrow), kGetCollisionArrow>();
    auto runtime = static_cast<ThrownWeapon*>(ctx->userData);
    if (runtime && runtime->OnHit(a_this, a_collector)) return;
    AttackHook::BeforeDamage(a_this, a_collector);
    original(a_this, a_collector);
}
RE::Projectile::ImpactData* ThrowableRelicWeapon::GetArrowImpactData(RE::ArrowProjectile* a_this, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable, std::uint32_t a6, std::uint32_t a7)
{
    auto ctx = ProjectileInstanceHook::From(a_this);
    if (!ctx) return nullptr;

    auto original = ctx->Original<decltype(&ThrowableRelicWeapon::GetArrowImpactData), kGetArrowImpactData>();
    auto runtime = static_cast<ThrownWeapon*>(ctx->userData);
    auto impactData = original(a_this, a_target, a_targetLoc, a_velocity, a_collidable, a6, a7);
    if (runtime)
        runtime->PostImpact(a_this, impactData, a_target, a_targetLoc, a_velocity, a_collidable);
    return impactData;
}
float ThrowableRelicWeapon::GetGravityArrow(RE::ArrowProjectile* a_this)
{
    auto ctx = ProjectileInstanceHook::From(a_this);
    if (!ctx) return a_this->GetProjectileBase() ? a_this->GetProjectileBase()->data.gravity : 1.f;

    auto original = ctx->Original<decltype(&ThrowableRelicWeapon::GetGravityArrow), kGetGravityArrow>();
    auto runtime = static_cast<ThrownWeapon*>(ctx->userData);
    if (runtime && runtime->projectile == a_this && !runtime->isDestroyed.load(std::memory_order_acquire) && !runtime->isAuxiliary)
        return runtime->currentGravity.load(std::memory_order_relaxed);
    return original(a_this);
}
void ThrowableRelicWeapon::OnEquip(BaseActor* a_actor)
{
    RelicWeapon::OnEquip(a_actor);
    if (a_actor && a_actor->IsValid()) {
        for (auto& [projectile, runtime] : thrownWeapons) {
            if (!runtime) continue;
            if (runtime->GetThrowState() != ThrowState::kArrived) {
                runtime->RemoveTrail();
                runtime->weaponParentNode.reset();
                runtime->replacedProjectileModel.reset();
            }
            runtime->ResetProjectile();
            runtime->SetThrowState(ThrowState::kThrowable);
            runtime->projState = ProjectileState::kNone;
        }

        auto weapIEDnew = ObjectUtil::Actor::GetInventoryEntryDataForBoundObject(wielder->GetActor(), weap);
        if (weapIEDnew && weapIEDnew->object) {
            weapIED = std::make_unique<RE::InventoryEntryData>(*weapIEDnew);
            weapIED->DeepCopy(*weapIEDnew);
        }
    } else {
        for (auto& [projectile, runtime] : thrownWeapons)
            if (runtime && runtime->GetThrowState() == ThrowState::kThrowable)
                runtime->SetThrowState(ThrowState::kNone);
    }
}
void ThrowableRelicWeapon::OnMenuOpenCloseEvent(const bool a_opening)
{
    if (a_opening) {
        GetSoundManager().PauseAllLoopingSounds();
        for (auto& [projectile, runtime] : thrownWeapons)
            if (runtime) runtime->GetSoundManager().PauseAllLoopingSounds();
    } else {
        GetSoundManager().ContinueAllLoopingSounds();
        for (auto& [projectile, runtime] : thrownWeapons)
            if (runtime) runtime->GetSoundManager().ContinueAllLoopingSounds();
    }
}
void ThrowableRelicWeapon::Update()
{
    ++updateCounter;
    GetSoundManager().Update();
    updateBuffer.clear();
    if (updateBuffer.capacity() < thrownWeapons.size())
        updateBuffer.reserve(thrownWeapons.size());
    for (auto& [projectile, runtime] : thrownWeapons)
        if (runtime) updateBuffer.emplace_back(runtime.get());
    for (auto runtime : updateBuffer)
        if (runtime) runtime->Update(*g_deltaTime);
    updateBuffer.clear();

    if (placeWeaponUpdate.IsTimeToUpdate()) {
        auto runtime = lastThrownWeapon;
        if (currentReference && runtime && runtime->replacedProjectileModel) {
            currentReference->Update3DPosition(true);
            currentReference->SetMotionType(RE::hkpMotion::MotionType::kKeyframed);
            if (auto droppedWeaponModel = currentReference->GetCurrent3D(); droppedWeaponModel && droppedWeaponModel->parent) {
                const auto& desiredParentWorldRotate = runtime->replacedProjectileModel->world.rotate;
                const auto& parentWorldRotate = droppedWeaponModel->parent->world.rotate;
                droppedWeaponModel->local.rotate = parentWorldRotate.Transpose() * desiredParentWorldRotate;
                currentReference->data.location = runtime->replacedProjectileModel->world.translate;
                RE::NiUpdateData updateData;
                updateData.flags.set(RE::NiUpdateData::Flag::kNone);
                updateData.time = 0.f;
                droppedWeaponModel->Update(updateData);
                placeWeaponUpdate.Done();
            }
        } else {
            placeWeaponUpdate.Done();
        }
    }

    CleanupThrownWeapons();
}
