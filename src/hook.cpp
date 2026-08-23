#include "hook.h"
#include "util.h"
#include "settings.h"
#include "MainKratosCombat.h"
using namespace Util;
using tState = LeviathanAxe::ThrowState;
using tStateM = Mjolnir::ThrowState;

static std::mutex ThrowCallMutex;
static double lastUpdate(0.0000);

/* //very good instance for projectile movement controlling from master ersh's true directional movement, 
void ProjectileHook::ProjectileAimSupport(RE::Projectile* a_this)
    {
        auto projectileNode = a_this->Get3D2();

        // player only, 0x100000 == player
        auto& shooter = a_this->GetProjectileRuntimeData().shooter;
        auto& desiredTarget = a_this->GetProjectileRuntimeData().desiredTarget;
        if (projectileNode && shooter.native_handle() == 0x100000) {
                if (!desiredTarget.get()) {
                    auto target = directionalMovementHandler->GetTarget();
                    auto targetPoint = directionalMovementHandler->GetTargetPoint();
                    if (!target || !targetPoint) {
                        return;
                    }
                    desiredTarget = target;
                    directionalMovementHandler->AddProjectileTarget(a_this->GetHandle(), targetPoint);
                
                if (aimType == TargetLockProjectileAimType::kHoming) {
                    // homing
                    auto targetPoint = directionalMovementHandler->GetProjectileTargetPoint(a_this->GetHandle());
                    if (targetPoint) {
                        RE::NiPoint3 handPos = targetPoint->world.translate;
                        auto& linearVelocity = a_this->GetProjectileRuntimeData().linearVelocity;
                        float speed = linearVelocity.Length();
                        //if (speed < 1500.f) {
                        //  return _GetLinearVelocity(a_this, a_outVelocity);
                        //}
                        RE::NiPoint3 direction = (handPos - leviPos);
                        // normalize direction
                        direction.Unitize();
                        // rotate
                        leviAngle.x = asin(direction.z);
                        leviAngle.z = atan2(direction.x, direction.y);
                        if (leviAngle.z < 0.0) {
                            leviAngle.z += PI;
                        }
                        if (direction.x < 0.0) {
                            leviAngle.z += PI;
                        }
                        SetRotationMatrix(projectileNode->local.rotate, -direction.x, direction.y, direction.z);
                        linearVelocity = direction * speed;
                    
                    }
                }
            }
        }
    }
*/
void ProjectileHook::GetLinearVelocityArrow(RE::ArrowProjectile* a_this, RE::NiPoint3& a_outVelocity)
{
    _GetLinearVelocityArrow(a_this, a_outVelocity);

    //  -this is required because this function is calling two times in one frame- but caused very weird bugs in combat state.
//    if (AsyncUtil::GameTime::GetEngineTime() - lastUpdate < (*g_deltaTimeRealTime / 10.f)) {
    //    spdlog::trace("duplication cancelled");
//        return;
//    } else {
//        lastUpdate = AsyncUtil::GameTime::GetEngineTime();
    //    spdlog::trace("delta t = {}, {}", *g_deltaTimeRealTime, *g_engineTime);
        if (ThrowCallMutex.try_lock()) {
            LeviAndDraupnir(a_this);
            ThrowCallMutex.unlock();
        }
//    }
}
void ProjectileHook::LeviAndDraupnir(RE::Projectile* a_this)
{
    auto& runtimeData = a_this->GetProjectileRuntimeData();
    const auto& shooter = runtimeData.shooter;

    if (shooter.native_handle() == 0x100000)    // player only, 0x100000 = player
    {
        auto projectileNode = a_this->Get3D2();
        if (!projectileNode) {
        //    spdlog::warn("projectile's 3d not loaded");
            return;
        }
        const auto projBase = a_this->GetProjectileBase();
        const auto& livingTime = runtimeData.livingTime;
        RE::Actor* AnArchos = shooter.get().get() ? shooter.get().get()->As<RE::Actor>() : nullptr;

        if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kLeviathanAxe)) {
            auto kratos = Kratos::GetSingleton();

            const auto targetPoint = WeaponIdentify::RHandBone;
            if (!targetPoint) {spdlog::warn("can't found your hand node for axe call!!"); return;}
            auto& handPos   = targetPoint->world.translate;
            auto& leviPos   = a_this->data.location;
            auto& leviAngle = a_this->data.angle;

            auto Levi = LeviathanAxe::GetSingleton();
            if (Levi->isAxeStucked) Levi->isAxeStucked = false;
            if (a_this != Levi->LastLeviProjectile) Levi->LastLeviProjectile = a_this;
            (projBase == Levi->LeviProjBaseL ? Levi->LeviathanAxeProjectileL : Levi->LeviathanAxeProjectileH) = a_this;
            (projBase == Levi->LeviProjBaseL ? Levi->LeviathanAxeProjectileH : Levi->LeviathanAxeProjectileL) = nullptr;

            if (Levi->data.model.get() != projectileNode) {
                Levi->data.model.reset(projectileNode);
                if (!Levi->isAxeCalled) {
                    Levi->soundData.PlayThrowingLoopSounds(projectileNode);
                }
            }
        //    for (auto& projTrail : Levi->data.projTrails) {
        //        if (projTrail) {
        //            projTrail->lifetime = projTrail->age + 0.1;
        //        }
        //    }
        //
        //    auto animatedBone = projectileNode->GetObjectByName("Cylinder02");
        //    auto leviModel = Levi->data.replacedProjectileModel;//(WeaponIdentify::LeviathanAxe->AsReference1() ? WeaponIdentify::LeviathanAxe->AsReference1()->Get3D() : nullptr);
        //    if (animatedBone && leviModel) {
        //        auto animatedNode = animatedBone->AsNode();
        //        auto leviNode = leviModel->AsNode();
        //        if (animatedNode && leviNode) {
        //            animatedNode->AttachChild(leviNode, false);
        //            spdlog::debug("levi model changed!");
        //        } else spdlog::debug("animated node or levinode null");
        //    } else spdlog::debug("animated bone or leviModel null");

            if (livingTime > 0.3f && Levi->GetThrowState() == tState::kThrown) Levi->SetThrowState(tState::kCanArrive);

            //speed
            auto& vel = runtimeData.linearVelocity;

            auto linearDir = vel;
            linearDir.Unitize();

            Levi->data.position = leviPos;
            Levi->data.lastOrientation = linearDir;
    //        if (APIs::Request()){// && livingTime < *g_deltaTimeRealTime) {
    //        //    auto rigidBody = NifUtil::Collision::GetRigidBody(projectileNode);
    //        //    RE::hkpRigidBody* hkpRigidBody = rigidBody ? static_cast<RE::hkpRigidBody*>(rigidBody->referencedObject.get()) : nullptr;
    //        //    hkpRigidBody->userData;
    //        //    APIs::precision->ApplyHitImpulse2(AnArchos->GetHandle(), AnArchos->GetHandle(), hkpRigidBody, vel, handPos, 10.f);
    //            APIs::precision->AddAttackTrail(projectileNode->AsNode(), AnArchos->GetHandle(), AnArchos->parentCell, a_this, PRECISION_API::TrailOverride());
    //        }
            if (!Levi->isAxeCalled) {
                Levi->data.lastVelocity = vel;
                Levi->data.throwedTime = livingTime;
                Levi->data.projState = LeviathanAxe::ProjectileState::kLaunched;

                if (Levi->IsHoming(a_this)) {
                    auto& hLevi = Levi->homingLevi;
                    float hLivingTime = hLevi.GetLivingTime();
                    auto hTarget = hLevi.GetNextTarget(leviPos);
                    if (hLivingTime > 1.2f && livingTime > 0.2f && hTarget) {
                        auto targetPos = hTarget->GetPosition() + (hTarget->GetBoundMax() + hTarget->GetBoundMin()) * 0.75f;
                        auto targetDir = (targetPos - leviPos);
                        float height = leviPos.z - hTarget->GetPosition().z;
                        targetDir.Unitize();
                        const float speed = hLevi.speed * (livingTime < 1.2f ? livingTime : 1.2f);
                        vel = MathUtil::Angle::BlendVectors(linearDir, targetDir, ((livingTime - 0.2f) / 3.f), true) * speed;
                        float dampFactor = std::clamp((height - 10.f) / 90.f, 0.f, 1.f);
                        if (vel.z < 0.f) vel.z *= dampFactor;  //  damp vertical speed
                    } else {
                        if (hLivingTime > 2.2f && livingTime > 0.3f && hLevi.targets.empty()) {
                            if (hLevi.isBoomerang && !WeaponIdentify::isRelic && !kratos->IsInRage(AnArchos)) kratos->DoKratosAction(Kratos::Action::kWeaponCharge, AnArchos);
                            else hLevi.proj = nullptr;  //  stop homing
                        }

                        // waving effect
                        float waveSin = hLevi.waveAmplitude * cos(hLevi.waveFrequency * hLivingTime);
                        float waveCos = hLevi.waveAmplitude * sin(hLevi.waveFrequency * hLivingTime);

                        auto targetPos = AnArchos->GetPosition() + (AnArchos->GetBoundMax() + AnArchos->GetBoundMin()) * 0.75f;
                        float distance = targetPos.GetDistance(leviPos);
                        float height = leviPos.z - AnArchos->GetPosition().z;
                        auto targetDir = targetPos - leviPos;
                        targetDir.Unitize();
                        RE::NiPoint3 circularVel;
                        RE::NiPoint3 originVelocity; AnArchos->GetLinearVelocity(originVelocity);
                        circularVel.x = -hLevi.speed * targetDir.y + originVelocity.x + waveSin;
                        circularVel.y = hLevi.speed * targetDir.x + originVelocity.y + waveCos;
                        circularVel.z = hLevi.speed * targetDir.z + originVelocity.z + waveCos;

                        circularVel += targetDir * (distance - 100.f) / 0.2f;
                        circularVel.Unitize();
                        const float speed = hLevi.speed * (hLivingTime > 0.5f && livingTime < 1.f ? livingTime + 0.2f : 1.f);
                        vel = MathUtil::Angle::BlendVectors(linearDir, circularVel, ((livingTime - 0.2f) / 3.f), true) * speed;
                        float dampFactor = std::clamp((height - 10.f) / 90.f, 0.f, 1.f);
                        if (vel.z < 0.f) vel.z *= dampFactor;  //  damp vertical speed
                    }

                    RE::NiPoint3 curvyDir = vel;
                    curvyDir.Unitize();
                    leviAngle.x = asin(curvyDir.z);
                    leviAngle.z = atan2(curvyDir.x, curvyDir.y);
                } else {
                //  skip gravity effect for a while
                    if (livingTime < Config::NoGravityDurationLeviathan) {
                        projBase->data.gravity = 0.f;
                    } else if (projBase->data.gravity == Levi->data.gravity) {
                        //  do nothing
                    } else {
                        projBase->data.gravity += *g_deltaTime * Levi->data.gravity;
                        if (projBase->data.gravity == Levi->data.gravity)
                            projBase->data.gravity = Levi->data.gravity;
                    }

                    RE::NiPoint3 rotation = linearDir;

                    leviAngle.x = asin(rotation.z);
                    leviAngle.z = atan2(rotation.x, rotation.y);
                }
            } else {
                if (projBase != Levi->LeviProjBaseA) {
            //        if (APIs::precision || APIs::Request()) {
            //            APIs::precision->RemoveProjectileCollision(AnArchos->GetHandle(), Levi->collisionDefinition);
            //        }
                    runtimeData.flags |= pFlag::kDestroyed;
                //  spdlog::debug("[HOOK] levi destroyed before call"); 
                    return;
                }
                if (Levi->GetThrowState() == tState::kCanArrive) Levi->SetThrowState(tState::kArriving);

                if (Levi->IsArriving(a_this)) {
                    auto& aLevi = Levi->arrivingLevi;
                    aLevi.Update(*g_deltaTimeRealTime);
                }
            }
        }
        else if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kMjolnir)) {
            auto kratos = Kratos::GetSingleton();

            const auto targetPoint = WeaponIdentify::RHandBone;
            if (!targetPoint) {spdlog::warn("can't found your hand node for Mjolnir call!!"); return;}
            auto& handPos   = targetPoint->world.translate;
            auto& mjolnirPos   = a_this->data.location;
            auto& mjolnirAngle = a_this->data.angle;

            auto mjolnir = Mjolnir::GetSingleton();
            if (a_this != mjolnir->LastMjolnirProjectile) mjolnir->LastMjolnirProjectile = a_this;
            mjolnir->MjolnirProjectileT = a_this;

            if (mjolnir->data.model.get() != projectileNode) {
                mjolnir->data.model.reset(projectileNode);
                if (!mjolnir->isMjolnirCalled) {
                    mjolnir->soundData.PlayThrowingLoopSounds(projectileNode);
                }
            }

            if (livingTime > 0.3f && mjolnir->GetThrowState() == tStateM::kThrown) mjolnir->SetThrowState(tStateM::kCanArrive);

            //speed
            auto& vel = runtimeData.linearVelocity;

            auto linearDir = vel;
            linearDir.Unitize();

            mjolnir->data.position = mjolnirPos;
            mjolnir->data.lastOrientation = linearDir;
            if (!mjolnir->isMjolnirCalled) {
                mjolnir->data.lastVelocity = vel;
                mjolnir->data.lastEulerAngles = mjolnirAngle;
                mjolnir->data.throwedTime = livingTime;
                mjolnir->data.projState = Mjolnir::ProjectileState::kLaunched;
            //    float speed = vel.Length();
            //    speed *= (*g_deltaTimeRealTime / *g_deltaTime); //  for not be effected by slow motion/fast motion
            //    vel = linearDir * speed;

            //  skip gravity effect for a while

                if (mjolnir->IsHoming(a_this)) {
                    auto& hMjolnir = mjolnir->homingMjolnir;
                    float hLivingTime = hMjolnir.GetLivingTime();
                    auto hTarget = hMjolnir.GetNextTarget(mjolnirPos);
                    if (hLivingTime > 1.2f && livingTime > 0.2f && hTarget) {
                        auto targetPos = hTarget->GetPosition() + (hTarget->GetBoundMax() + hTarget->GetBoundMin()) * 0.75f;
                        auto targetDir = (targetPos - mjolnirPos);
                        float height = mjolnirPos.z - hTarget->GetPosition().z;
                        targetDir.Unitize();
                        const float speed = hMjolnir.speed * (livingTime < 1.2f ? livingTime : 1.2f);
                        vel = MathUtil::Angle::BlendVectors(linearDir, targetDir, ((livingTime - 0.2f) / 3.f), true) * speed;
                        float dampFactor = std::clamp((height - 10.f) / 90.f, 0.f, 1.f);
                        if (vel.z < 0.f) vel.z *= dampFactor;  //  damp vertical speed
                    } else {
                        if (hLivingTime > 2.2f && livingTime > 0.3f && hMjolnir.targets.empty()) {
                            if (hMjolnir.isBoomerang && !WeaponIdentify::isRelic && !kratos->IsInRage(AnArchos)) kratos->DoKratosAction(Kratos::Action::kWeaponCharge, AnArchos);
                            else hMjolnir.proj = nullptr;  //  stop homing
                        }

                        // waving effect
                        float waveSin = hMjolnir.waveAmplitude * cos(hMjolnir.waveFrequency * hLivingTime);
                        float waveCos = hMjolnir.waveAmplitude * sin(hMjolnir.waveFrequency * hLivingTime);

                        auto targetPos = AnArchos->GetPosition() + (AnArchos->GetBoundMax() + AnArchos->GetBoundMin()) * 0.75f;
                        float distance = targetPos.GetDistance(mjolnirPos);
                        float height = mjolnirPos.z - AnArchos->GetPosition().z;
                        auto targetDir = targetPos - mjolnirPos;
                        targetDir.Unitize();
                        RE::NiPoint3 circularVel;
                        RE::NiPoint3 originVelocity; AnArchos->GetLinearVelocity(originVelocity);
                        circularVel.x = -hMjolnir.speed * targetDir.y + originVelocity.x + waveSin;
                        circularVel.y = hMjolnir.speed * targetDir.x + originVelocity.y + waveCos;
                        circularVel.z = hMjolnir.speed * targetDir.z + originVelocity.z + waveCos;

                        circularVel += targetDir * (distance - 100.f) / 0.2f;
                        circularVel.Unitize();
                        const float speed = hMjolnir.speed * (hLivingTime > 0.5f && livingTime < 1.f ? livingTime + 0.2f : 1.f);
                        vel = MathUtil::Angle::BlendVectors(linearDir, circularVel, ((livingTime - 0.2f) / 3.f), true) * speed;
                        float dampFactor = std::clamp((height - 10.f) / 90.f, 0.f, 1.f);
                        if (vel.z < 0.f) vel.z *= dampFactor;  //  damp vertical speed
                    }

                    RE::NiPoint3 curvyDir = vel;
                    curvyDir.Unitize();
                    mjolnirAngle.x = asin(curvyDir.z);
                    mjolnirAngle.z = atan2(curvyDir.x, curvyDir.y);
                } else {
                    if (livingTime < Config::NoGravityDurationMjolnir) {
                        projBase->data.gravity = 0.f;
                    } else if (projBase->data.gravity == mjolnir->data.gravity) {
                        //  do nothing
                    } else {
                        projBase->data.gravity += *g_deltaTime * mjolnir->data.gravity;
                        if (projBase->data.gravity == mjolnir->data.gravity)
                            projBase->data.gravity = mjolnir->data.gravity;
                    }
                    //apply rotation
                    float rot = mjolnir->data.rotationSpeed * livingTime;
                    mjolnirAngle.x = asin(linearDir.z);
            //        mjolnirAngle.y = mjolnir->data.yAngle;
                    mjolnirAngle.y = rot;
                }
            } else {
            //  float passedArrTime = livingTime - mjolnir->throwedTime;
            //  if (passedArrTime < 0.f) passedArrTime = livingTime;
                float arrivingTime = Config::ArrivalTime - livingTime;//passedArrTime;
                if (arrivingTime < *g_deltaTime * 2.f) arrivingTime = *g_deltaTime * 2.f;

                const float distance = handPos.GetDistance(mjolnirPos);
                mjolnir->arrivingMjolnir.linearDistance = distance;
                auto linearArrivingDir = handPos - mjolnirPos;
                linearArrivingDir.Unitize();
                mjolnir->arrivingMjolnir.linearArrivingDir = linearArrivingDir;
                mjolnir->arrivingMjolnir.currentDir = linearDir;

            //  speed calculation
                float arrSpeed = distance / arrivingTime;

            //  set speed limits
                const bool isCatchable = (distance <= Config::CatchingTreshold) || (distance <= (*g_deltaTime * vel.Length()));
                if (arrSpeed < Config::MinArrivalSpeed && !isCatchable) arrSpeed = Config::MinArrivalSpeed * 0.7f;
                else if (arrSpeed > Config::MaxArrivalSpeed)            arrSpeed = Config::MaxArrivalSpeed * 0.7f;

                if (mjolnir->isMjolnirArriving) {
                    if (projBase != mjolnir->MjolnirProjBaseA) {
                        runtimeData.flags |= pFlag::kDestroyed;
                    //  spdlog::debug("[HOOK] mjolnir destroyed before call");
                        return;
                    }
                    if (isCatchable && projBase == mjolnir->MjolnirProjBaseA) {
                        if (mjolnir->GetThrowState() == tStateM::kArriving) mjolnir->SetThrowState(tStateM::kArrived);
                        mjolnir->Catch();
                        spdlog::debug("Mjolnir proj catched");
                //    } else {
                //        a_this->GetCurrent3D()->world.rotate.SetEulerAnglesXYZ(40,30,60); //  these methods can't change rotation
                //        a_this->Get3D()->world.rotate.SetEulerAnglesXYZ(40,30,60);
                //        a_this->Get3D1(false)->world.rotate.SetEulerAnglesXYZ(40,30,60);
                //        a_this->Get3D2()->world.rotate.SetEulerAnglesXYZ(40,30,60);
                    }
                    if (mjolnir->GetThrowState() == tStateM::kCanArrive) mjolnir->SetThrowState(tStateM::kArriving);

                    if (mjolnir->IsArriving(a_this)) {
                        auto& aMjolnir = mjolnir->arrivingMjolnir;
                        aMjolnir.Update(*g_deltaTimeRealTime);
                    }
                } else if (Config::MjolnirArrivingDelay.has_value()) {  // blending state
                    auto& aMjolnir = mjolnir->arrivingMjolnir;
                    const float blendTime = *Config::MjolnirArrivingDelay;
                    auto velocityWhenCalled = mjolnir->data.projState == Mjolnir::ProjectileState::kLaunched ? mjolnir->data.lastVelocity : (aMjolnir.linearArrivingDir * aMjolnir.speed);
                    vel = MathUtil::Angle::BlendVectors(velocityWhenCalled, linearArrivingDir * mjolnir->arrivingMjolnir.speed, mjolnir->arrivingMjolnir.GetLivingTime() / blendTime);
                    aMjolnir.startVelocity = vel;
                    RE::NiPoint3 curvyDir = vel;
                    curvyDir.Unitize();
                    mjolnirAngle = mjolnir->data.lastEulerAngles;  //  keep last angle
                    mjolnirAngle.x = asin(curvyDir.z) + PI2;
                    mjolnirAngle.z = atan2(curvyDir.x, curvyDir.y);
                    mjolnir->data.lastEulerAngles = mjolnirAngle;
                    aMjolnir.UpdateArrivingDirection(*g_deltaTimeRealTime);
                }
            }
        }
        else if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kDraupnirSpear)) {
            if (projBase == Draupnir::DraupnirSpearProjBaseL) {
                Draupnir::data.model.reset(projectileNode);
            //  skip gravity effect for a while
                if (livingTime < Config::NoGravityDurationDraupnir) {
                    projBase->data.gravity = 0.f;
                } else if (projBase->data.gravity == Draupnir::data.gravity) {
                    //  do nothing
                } else {
                    projBase->data.gravity += *g_deltaTime * Draupnir::data.gravity;
                    if (projBase->data.gravity == Draupnir::data.gravity)
                        projBase->data.gravity = Draupnir::data.gravity;
                }

                auto& mjolnirAngle = a_this->data.angle;
                float rot = Draupnir::data.rotationSpeed * livingTime;

                auto& vel = runtimeData.linearVelocity;
                auto linearDir = vel;
                linearDir.Unitize();

                mjolnirAngle.x = asin(linearDir.z);
                mjolnirAngle.y = rot;

                for (auto ID : Draupnir::MeleeHitProjectileIDs) {
                    if (ID == a_this->formID) {
                        if (livingTime > 0.069f) {
                            runtimeData.flags|= pFlag::kDestroyed;
                        //    Draupnir::DraupnirSpearProjBaseL->model = Draupnir::DefaultDraupnirModel;
                            Draupnir::MeleeHitProjectileIDs.clear();
                        }
                    }
                }
            }
        //    else if (projBase == Draupnir::DraupnirsCallProjBaseL) {
        //        runtimeData.explosionTimer = 0.f;
        //    }
    }
#ifdef TRIDENT
        else if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kTrident)) {
            if (projBase == Trident::GetSingleton()->TridentCallProjBaseL) {}
            else {
                auto trident = Trident::GetSingleton();
                trident->data.proj = a_this;
                trident->data.model = projectileNode;
                trident->data.position = a_this->data.location;
            }
        }
#endif
    }
}

void ProjectileHook::GetCollisionArrow(RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
    if (LeviAndDraupnirHit(a_this, a_AllCdPointCollector)) return;
    AttackHook::BeforeDamage(a_this, a_AllCdPointCollector);
    _GetCollisionArrow(a_this, a_AllCdPointCollector);
}
void ProjectileHook::GetCollisionMissile(RE::MissileProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
    AttackHook::BeforeDamage(a_this, a_AllCdPointCollector);
    _GetCollisionMissile(a_this, a_AllCdPointCollector);
}
inline bool ProjectileHook::LeviAndDraupnirHit(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
    const auto projBase = a_this->GetProjectileBase();
    if (projBase && WeaponIdentify::IsRelic(projBase)) {

        auto& rtData = a_this->GetProjectileRuntimeData();
        RE::Actor* shooter = nullptr;
        if (rtData.shooter && rtData.shooter.get() && rtData.shooter.get().get())   shooter = rtData.shooter.get().get()->As<RE::Actor>();

        if (auto Levi = LeviathanAxe::GetSingleton(); WeaponIdentify::IsRelic(projBase, Kratos::Relic::kLeviathanAxe)) {
            if (/*rtData.livingTime < 0.01f || */(Levi->isAxeCalled && Config::DontDamageWhileArrive)) return true;

            for (auto& point : a_AllCdPointCollector->hits) {
            //    const auto ourProj  = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableA);
                const auto target   = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

                const bool isHoming = Levi->IsHoming(a_this);// && (mjolnir->homingMjolnir.GetNextTarget() || mjolnir->homingMjolnir.GetLivingTime() < 1.f);
                const bool isArriving = projBase == Levi->LeviProjBaseA;
                bool isSameTarget = false;
                bool isTargetActor = target ? target->formType == RE::FormType::ActorCharacter : false;
                const auto victim = target ? target->As<RE::Actor>() : nullptr;
            //  if (ourProj) {
            //      if (projBase == Levi->LeviProjBaseL) a_this->data.angle.z = Config::MaxAxeStuckAngle;
            //      else a_this->data.angle.y = Config::MaxAxeStuckAngle;
            //  }
                if (target && a_this != target->AsProjectile()) {
                    if (!isTargetActor && !Levi->data.lastHitForms.empty() && std::find(Levi->data.lastHitForms.begin(), Levi->data.lastHitForms.end(), target) != Levi->data.lastHitForms.end()) isSameTarget = true;
                    else if (victim && victim != shooter) {
                        if (a_this->IsMissileProjectile()) {
                            if (!Levi->data.lastHitActors.empty() && std::find(Levi->data.lastHitActors.begin(), Levi->data.lastHitActors.end(), victim) != Levi->data.lastHitActors.end()) isSameTarget = true;
                            if (isHoming && victim == Levi->homingLevi.GetNextTarget() && isSameTarget && rtData.livingTime > 0.5f) isSameTarget = false;
                            if (isArriving && victim == Levi->arrivingLevi.GetNextTarget() && isSameTarget && rtData.livingTime > 0.5f) isSameTarget = false;
                            if (!victim->IsDead()) {
                                if (!isArriving) Levi->data.stuckedActor.reset(victim);

                                if (Levi->data.weap && Levi->data.ench && Levi->data.ench->effects[0])
                                    ObjectUtil::Enchantment::ChargeInventoryWeapon(shooter, Levi->data.weap, -Levi->data.ench->effects[0]->effectItem.magnitude);

                                if (shooter && !isArriving) {
                                    shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);

                                    auto kratos = Kratos::GetSingleton();
                                    kratos->RestoreRage(shooter, kratos->CalcRageDamageOrBuffAmount(rtData.weaponDamage, 0.5f));
                                }
                            }
                        }
                    }
                } else {spdlog::warn("WEIRD, target or levi is not exists");}
                if (isSameTarget || (!isTargetActor && (isArriving || isHoming))) {
                    return true;
                } else {
                    Levi->trailUpdate.Done();
                    Levi->trailRemoveUpdate.RegisterForUpdate(0.f, false);
                    if (const auto model = a_this->Get3D(); model) {
                        auto controllers = model->GetControllers();
                        if (controllers) {
                            auto manager = controllers->AsNiControllerManager();
                            if (manager) {
                                auto endSequence = manager->GetSequenceByName("mEnd");
                                if (endSequence && shooter) shooter->PlayAnimation(manager, endSequence, endSequence);
                            }
                        }
                    }
                }

#ifdef EXPERIMENTAL
            //  const bool isVertical = !isArriving && projBase == Levi->LeviProjBaseH;
            //  auto offset = projBase->data.collisionRadius;
            //  auto& pos = a_this->data.location;
            //  auto& angle = a_this->data.angle;
            //  auto& matrixL = a_this->Get3D2()->local.rotate;
            //  auto& matrixW = a_this->Get3D2()->world.rotate;
            //  auto hitDir = rtData.linearVelocity;
            //  hitDir.Unitize();
            //  Levi->SetHitRotation(angle, hitDir, isVertical);
            //  Levi->TweakHitPosition(pos, hitDir, offset, isVertical);
            //  Levi->SetHitRotation(matrixL, isVertical);
            //  Levi->SetHitRotation(matrixW, isVertical);
#endif
            }

            if (Levi->GetThrowState() == tState::kThrown) Levi->SetThrowState(tState::kCanArrive);
        }
        else if (auto mjolnir = Mjolnir::GetSingleton(); WeaponIdentify::IsRelic(projBase, Kratos::Relic::kMjolnir)) {
            if (/*rtData.livingTime < 0.01f || */(mjolnir->isMjolnirCalled && Config::DontDamageWhileArrive)) return true;

            for (auto& point : a_AllCdPointCollector->hits) {
                auto target   = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

                mjolnir->data.isPenetrating = false;
                const bool isHoming = mjolnir->IsHoming(a_this);// && (mjolnir->homingMjolnir.GetNextTarget() || mjolnir->homingMjolnir.GetLivingTime() < 1.f);
                const bool isArriving = projBase == mjolnir->MjolnirProjBaseA;
                bool isSameTarget = false;
                bool isTargetActor = target ? target->formType == RE::FormType::ActorCharacter : false;
                const auto victim = target ? target->As<RE::Actor>() : nullptr;
                if (target && a_this != target->AsProjectile()) {
                    if (!isTargetActor && !mjolnir->data.lastHitForms.empty() && std::find(mjolnir->data.lastHitForms.begin(), mjolnir->data.lastHitForms.end(), target) != mjolnir->data.lastHitForms.end()) isSameTarget = true;
                    else if (victim && victim != shooter) {
                        if (a_this->IsMissileProjectile()) {
                            if (!mjolnir->data.lastHitActors.empty() && std::find(mjolnir->data.lastHitActors.begin(), mjolnir->data.lastHitActors.end(), victim) != mjolnir->data.lastHitActors.end()) isSameTarget = true;
                            if (isHoming && victim == mjolnir->homingMjolnir.GetNextTarget() && isSameTarget && rtData.livingTime > 0.5f) isSameTarget = false;
                            if (isArriving && victim == mjolnir->arrivingMjolnir.GetNextTarget() && isSameTarget && rtData.livingTime > 0.5f) isSameTarget = false;
                            if (!victim->IsDead()) {
                                if (mjolnir->data.weap && mjolnir->data.ench && mjolnir->data.ench->effects[0])
                                    ObjectUtil::Enchantment::ChargeInventoryWeapon(shooter, mjolnir->data.weap, -mjolnir->data.ench->effects[0]->effectItem.magnitude);

                                auto kratos = Kratos::GetSingleton();
                                if (shooter && !isArriving) {
                                    shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);

                                    kratos->RestoreRage(shooter, kratos->CalcRageDamageOrBuffAmount(rtData.weaponDamage, 0.5f));
                                }
#ifdef EXPERIMENTAL_PROJECTILE_HIT_EXPLOSION
                                mjolnir->data.isPenetrating = true;
                                auto weaponIE = ObjectUtil::Actor::GetInventoryEntryDataForBoundObject(shooter, mjolnir->data.weap);
                                static RE::HitData hitData;
                                hitData.Populate(shooter, victim, weaponIE);

                                RE::NiPoint3 hitDir = rtData.linearVelocity - (victim->GetPosition() + (victim->GetBoundMax() + victim->GetBoundMin()) * 0.75f);
                                hitDir.Unitize();
                                RE::Projectile::ProjectileRot pRot = MathUtil::Algebra::VectorToPitchYaw(hitDir);
                                RE::NiPoint3 origin = a_this->data.location;
                                RE::Projectile::LaunchData lData(shooter, origin, pRot, kratos->vanillaSpell.lightningBolt);
                                lData.autoAim = false;
                                lData.alwaysHit = true;
                                lData.desiredTarget = target;
                                RE::ProjectileHandle pHandle;
                                RE::Projectile::Launch(&pHandle, lData);

                            //    if (auto victimAVO = victim->AsActorValueOwner(); victimAVO) {
                            //        if (hitData.totalDamage > (victimAVO->GetActorValue(RE::ActorValue::kHealth) / 10.f)) {
                            //            RE::NiPoint3 hitDir;// = rtData.linearVelocity;
                            //            hitDir.Unitize();
                            //            ObjectUtil::Actor::PushActorAway(victim, rtData.power, hitDir);
                            //        } else FenixUtils::stagger(1.f, victim, a_this);
                            //        victimAVO->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -hitData.totalDamage);
                            //    }
                                mjolnir->data.lastHitActors.insert(mjolnir->data.lastHitActors.begin(), victim); 
                                if (mjolnir->data.lastHitActors.size() > 4) mjolnir->data.lastHitActors.pop_back();
                                RE::hkVector4 pos(a_this->data.location);
                                RE::TESHavokUtilities::AddExplosionImpulse(a_this->Get3D2(), pos, 1000.f, &hitData);
                                spdlog::debug("!mjolnir caused explosion impulse to target: {} by {} damage", target->GetName(), hitData.totalDamage);
                                return true;
#endif

                            }
                        }
                    }
                } else {spdlog::warn("WEIRD, target or mjolnir is not exists");}
                if (isSameTarget || (!isTargetActor && (isArriving || isHoming))) {
                    return true;
                } else {
                    mjolnir->trailUpdate.Done();
                    mjolnir->trailRemoveUpdate.RegisterForUpdate(0.f, false);
                }
            }

            if (mjolnir->GetThrowState() == tStateM::kThrown) mjolnir->SetThrowState(tStateM::kCanArrive);
        }
        else if (projBase == Draupnir::DraupnirSpearProjBaseL) {

            Draupnir::ReplaceStickedProjectileModel(a_this);

        //    RE::NiPointer<RE::BSTempEffectParticle> particleEffect;
        //    std::string modelPath = "D:\\Games\\Skyrim Modlists\\Anvil\\mods\\Kratos Combat - Draupnir - v1.6\\meshes\\weapons\\DraupnirSpear\\StuckedDraupnirSpear.nif";
        //    auto model = a_this->Get3D();
        //    if (model) {
        //        particleEffect.reset(RE::BSTempEffectParticle::Spawn(
        //            shooter->GetParentCell(),
        //            10.f,
        //            modelPath.data(),
        //            model->world.rotate,
        //            model->world.translate,
        //            10.f,
        //            7,
        //            model->AsNode()));
        //    }
        //    if (particleEffect && particleEffect->AsNode()) {
        //    //    model->AsNode()->AttachChild(particleEffect->AsNode(), false);
        //    }
            Draupnir::data.replacedProjectileModel.reset();

            for (auto& point : a_AllCdPointCollector->hits) {
                const auto target = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

                if (target) {
                    if (target->formType == RE::FormType::ActorCharacter) {
                        if (const auto victim = target->As<RE::Actor>(); victim) {
                            if (!victim->IsDead()) {
                                if (Draupnir::data.weap && Draupnir::data.ench && Draupnir::data.ench->effects[0])
                                    ObjectUtil::Enchantment::ChargeInventoryWeapon(shooter, Draupnir::data.weap, -Draupnir::data.ench->effects[0]->effectItem.magnitude);

                                if (shooter) {
                                    shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);

                                    auto kratos = Kratos::GetSingleton();
                                    kratos->RestoreRage(shooter, kratos->CalcRageDamageOrBuffAmount(rtData.weaponDamage, 0.5f));
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (projBase == Draupnir::DraupnirsCallProjBaseL) {
            return true;
        }
#ifdef TRIDENT
/**/
        else if (auto trident = Trident::GetSingleton(); WeaponIdentify::IsRelic(projBase, Kratos::Relic::kTrident)) {
            if (projBase == trident->TridentCallProjBaseL) return true;

            for (auto& point : a_AllCdPointCollector->hits) {
                const auto target   = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

                bool isSameTarget = false;
                bool isTargetActor = target ? target->formType == RE::FormType::ActorCharacter : false;
                const auto victim = target ? target->As<RE::Actor>() : nullptr; 
                if (target && victim != shooter && a_this != target->AsProjectile()) {
                    if (!isTargetActor && !trident->data.lastHitForms.empty() && std::find(trident->data.lastHitForms.begin(), trident->data.lastHitForms.end(), target) != trident->data.lastHitForms.end()) isSameTarget = true;
                    else if (victim) {
                        if (a_this->IsMissileProjectile()) {
                            if (!trident->data.lastHitActors.empty() && std::find(trident->data.lastHitActors.begin(), trident->data.lastHitActors.end(), victim) != trident->data.lastHitActors.end()) isSameTarget = true;
                            else if (!target->IsDead()) {
                                isTargetActor = true;
                                if (trident->data.weap && trident->data.ench && trident->data.ench->effects[0])
                                    ObjectUtil::Enchantment::ChargeInventoryWeapon(shooter, trident->data.weap, -trident->data.ench->effects[0]->effectItem.magnitude);

                                if (shooter)
                                    shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);

                                    auto kratos = Kratos::GetSingleton();
                                    kratos->RestoreRage(shooter, kratos->CalcRageDamageOrBuffAmount(rtData.weaponDamage, 0.5f));
                            }
                        }
                    }
                } else spdlog::warn("WEIRD, target or Trident is not exists");
                if (isSameTarget) return true;
            }
        }
#endif
    }
    return false;
}

RE::Projectile::ImpactData* ProjectileHook::GetMissileImpactData(RE::MissileProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, uint32_t a7)
{
    auto impactData = _GetMissileImpactData(proj, a_target, a_targetLoc, a_velocity, a_collidable, a6, a7);
//    LeviAndDraupnirImpactData(impactData, proj, a_target, a_targetLoc, a_velocity, a_collidable);
    return impactData;
}
RE::Projectile::ImpactData* ProjectileHook::GetArrowImpactData(RE::ArrowProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, uint32_t a7)
{
    auto impactData = _GetArrowImpactData(proj, a_target, a_targetLoc, a_velocity, a_collidable, a6, a7);
    LeviAndDraupnirImpactData(impactData, proj, a_target, a_targetLoc, a_velocity, a_collidable);
    return impactData;
}
void ProjectileHook::LeviAndDraupnirImpactData(RE::Projectile::ImpactData* impactData, RE::MissileProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable)
{
    if (proj && impactData) {
        const bool isTargetActor = a_target ? a_target->formType == RE::FormType::ActorCharacter : false;
        const auto ActorTarget = isTargetActor ? a_target->As<RE::Actor>() : nullptr;
        auto& rtData = proj->GetProjectileRuntimeData();
        auto& missileRTD = proj->GetMissileRuntimeData();
        auto shooter = rtData.shooter ? rtData.shooter.get()->As<RE::Actor>() : nullptr;
        const auto projBase = proj->GetProjectileBase();
#ifdef TRIDENT
        auto trident = Trident::GetSingleton();
#endif

        if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kLeviathanAxe)) {
            auto Levi = LeviathanAxe::GetSingleton();
            Levi->trailUpdate.Done();
            Levi->LastLeviProjectile = proj;
            bool stopSounds = true;
            if (isTargetActor)  {Levi->data.lastHitActors.insert(Levi->data.lastHitActors.begin(), ActorTarget); if (Levi->data.lastHitActors.size() > 4) Levi->data.lastHitActors.pop_back();}
            else if (a_target)  {Levi->data.lastHitForms.insert(Levi->data.lastHitForms.begin(), a_target); if (Levi->data.lastHitForms.size() > 4) Levi->data.lastHitForms.pop_back();}
            if (projBase == Levi->LeviProjBaseA) {
                spdlog::debug("levi impacted to {} while arriving", a_target ? a_target->GetName() : "NULL");
                if (isTargetActor && ActorTarget == Levi->arrivingLevi.GetNextTarget()) Levi->arrivingLevi.targets.erase(Levi->arrivingLevi.targets.begin());
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                impactData->impactResult = RE::ImpactResult::kBounce;
                Levi->Call(false, true);
            } else if (projBase == Levi->LeviProjBaseL) {
                if (isTargetActor) {
                    Levi->data.projState = LeviathanAxe::ProjectileState::kHavok;
                    missileRTD.impactResult = RE::ImpactResult::kBounce;
                    impactData->impactResult = RE::ImpactResult::kBounce;
                }
            }

            const bool itWillStick = (impactData->impactResult == RE::ImpactResult::kStick || missileRTD.impactResult == RE::ImpactResult::kStick);
            if (const bool isHoming = Levi->IsHoming(proj); isHoming) {
                if (isHoming && isTargetActor && ActorTarget == Levi->homingLevi.GetNextTarget()) Levi->homingLevi.targets.erase(Levi->homingLevi.targets.begin());
                Levi->data.projState = LeviathanAxe::ProjectileState::kNone;
                missileRTD.impactResult = RE::ImpactResult::kDestroy;
                impactData->impactResult = RE::ImpactResult::kDestroy;
                Levi->Throw(false, true, isHoming);
                stopSounds = false;
                spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
            } else if (itWillStick) {
        //        if (isTargetActor && rtData.weaponDamage > (ActorTarget->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) / 10.f)/**/) {
        //            ObjectUtil::Actor::PushActorAway(ActorTarget, projBase->data.force, rtData.linearVelocity);
        //        }
                Levi->data.stuckedActor.reset(ActorTarget);
                Levi->data.stuckedBone.reset(impactData->damageRootNode);
                Levi->isAxeStucked = true;
                Levi->data.projState = LeviathanAxe::ProjectileState::kStucked;
                impactData->impactResult = RE::ImpactResult::kStick;
                missileRTD.impactResult = RE::ImpactResult::kStick;
                spdlog::debug("{} is sticked to {}!", projBase->GetName(), Levi->data.stuckedBone ? Levi->data.stuckedBone->name : "NULL");
            } else {
                Levi->data.projState = LeviathanAxe::ProjectileState::kHavok;
                impactData->impactResult = RE::ImpactResult::kBounce;
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                spdlog::debug("{} is bounced from {}!", projBase->GetName(), Levi->data.stuckedBone ? Levi->data.stuckedBone->name : "NULL");
            }
            if (stopSounds)
                Levi->soundData.FadeThrowingLoopSounds(369);

        //    if (shooter && (APIs::precision || APIs::Request())) {
        //        APIs::precision->RemoveProjectileCollision(shooter->GetHandle(), Levi->collisionDefinition);
        //    }
        //  const bool isVertical = projBase == Levi->LeviProjBaseH;
        //  auto offset = projBase->data.collisionRadius;
        //  auto& pos = proj->data.location;
        //  auto& angle = proj->data.angle;
        //  auto& matrixL = proj->Get3D2()->local.rotate;
        //  auto& matrixW = proj->Get3D2()->world.rotate;
        //  auto& rtData = proj->GetProjectileRuntimeData();
        //  auto hitDir = rtData.linearVelocity;
        //  hitDir.Unitize();
        //  Levi->SetHitRotation(angle, hitDir, isVertical);
        //  Levi->TweakHitPosition(pos, hitDir, offset, isVertical);
        //  Levi->SetHitRotation(matrixL, isVertical);
        //  Levi->SetHitRotation(matrixW, isVertical);
        //    spdlog::debug("hit angle = [{}, {}, {}]", proj->data.angle.x, proj->data.angle.y, proj->data.angle.z);
        }
        else if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kMjolnir)) {
            auto mjolnir = Mjolnir::GetSingleton();
            mjolnir->trailUpdate.Done();
            bool skipIt = false;
            bool stopSounds = true;
            if (isTargetActor)  {mjolnir->data.lastHitActors.insert(mjolnir->data.lastHitActors.begin(), ActorTarget); if (mjolnir->data.lastHitActors.size() > 4) mjolnir->data.lastHitActors.pop_back();}
            else if (a_target)  {mjolnir->data.lastHitForms.insert(mjolnir->data.lastHitForms.begin(), a_target); if (mjolnir->data.lastHitForms.size() > 4) mjolnir->data.lastHitForms.pop_back();}
            else skipIt = true;
            if (projBase == mjolnir->MjolnirProjBaseA) {
                spdlog::debug("mjolnir impacted to {} while arriving", a_target ? a_target->GetName() : "NULL");
                if (isTargetActor && ActorTarget == mjolnir->arrivingMjolnir.GetNextTarget()) mjolnir->arrivingMjolnir.targets.erase(mjolnir->arrivingMjolnir.targets.begin());
                missileRTD.impactResult = RE::ImpactResult::kDestroy;
                impactData->impactResult = RE::ImpactResult::kDestroy;
                mjolnir->Call(false, true);
            } else {
            //    if (isTargetActor && rtData.weaponDamage > (ActorTarget->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) / 20.f))
            //        ObjectUtil::Actor::PushActorAway(ActorTarget, projBase->data.force);
            //    const bool itWillStick = (impactData->impactResult == RE::ImpactResult::kStick || missileRTD.impactResult == RE::ImpactResult::kStick);
                bool itWillBounce = (impactData->impactResult == RE::ImpactResult::kBounce || missileRTD.impactResult == RE::ImpactResult::kBounce);
                const bool isHoming = mjolnir->IsHoming(proj);
                if (skipIt && !isHoming) {
                    mjolnir->data.projState = Mjolnir::ProjectileState::kHavok;
                    missileRTD.impactResult = RE::ImpactResult::kBounce;
                    impactData->impactResult = RE::ImpactResult::kBounce;
                    itWillBounce = true;
                    spdlog::debug("mjolnir hit an unidentified object and bounced!");
                } else if (!itWillBounce || isTargetActor || isHoming) {
                    if (isHoming && isTargetActor && ActorTarget == mjolnir->homingMjolnir.GetNextTarget()) mjolnir->homingMjolnir.targets.erase(mjolnir->homingMjolnir.targets.begin());
                    mjolnir->data.projState = Mjolnir::ProjectileState::kNone;
                    missileRTD.impactResult = RE::ImpactResult::kDestroy;
                    impactData->impactResult = RE::ImpactResult::kDestroy;
                    itWillBounce = false;
                    mjolnir->Throw(true, false, isHoming);
                    stopSounds = false;
                    spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
                } else {
                    mjolnir->data.projState = Mjolnir::ProjectileState::kHavok;
                    spdlog::debug("{} is bounced from {}!", projBase->GetName(), a_target ? a_target->GetName() : "NULL");
                }
            }
            if (stopSounds)
                mjolnir->soundData.FadeThrowingLoopSounds(369);
        }
        else if (projBase == Draupnir::DraupnirsCallProjBaseL) {
            missileRTD.impactResult = RE::ImpactResult::kImpale;
            impactData->impactResult = RE::ImpactResult::kImpale;
        }
        else if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kDraupnirSpear)) {
        //    bool isMelee = false;
        //    if (!Draupnir::MeleeHitProjectileIDs.empty())
        //        for (auto ID : Draupnir::MeleeHitProjectileIDs) {
        //            if (ID == proj->formID) {
        //            //    proj->Set3D(Draupnir::spearModel, false);
        //                isMelee = true;
        //            }
        //        }
        //    if (!isMelee) Draupnir::spearModel = proj->Get3D();
            auto impactDataP = *rtData.impacts.begin();
            impactDataP->impactResult = RE::ImpactResult::kStick;
            missileRTD.impactResult = RE::ImpactResult::kStick;
    //        impactData->impactResult = RE::ImpactResult::kStick;
            const bool itWillStick = (missileRTD.impactResult == RE::ImpactResult::kStick);
            spdlog::debug("draupnir is will {}", itWillStick ? "stick" : "bounce");
#ifdef NEW_DRAUPNIR_EXPLOSION_METHOD
            if (ActorTarget && impactData->damageRootNode && itWillStick) {
                Draupnir::AddSpearHit(impactData->damageRootNode, ActorTarget);
                spdlog::debug("spear stucked to {} of {}!", impactData->damageRootNode->name, ActorTarget->GetName());
            } else {
                Draupnir::AddSpearHit(proj);
                spdlog::debug("draupnir's target is not an actor.");
            }
#endif
        }
#ifdef TRIDENT
        else if (projBase == trident->TridentCallProjBaseL) {
        //    missileRTD.impactResult = RE::ImpactResult::kImpale;
        //    impactData->impactResult = RE::ImpactResult::kImpale;
        }
        else if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kTrident)) {
            if (isTargetActor)  {trident->data.lastHitActors.insert(trident->data.lastHitActors.begin(), ActorTarget); if (trident->data.lastHitActors.size() > 3) trident->data.lastHitActors.pop_back();}
            else if (a_target)  {trident->data.lastHitForms.insert(trident->data.lastHitForms.begin(), a_target); if (trident->data.lastHitForms.size() > 3) trident->data.lastHitForms.pop_back();}
            missileRTD.impactResult = RE::ImpactResult::kImpale;
            impactData->impactResult = RE::ImpactResult::kImpale;
            if (projBase != trident->TridentCallProjBaseL) {
                if (isTargetActor && impactData->damageRootNode) {
                    trident->data.lastHitBone = impactData->damageRootNode;
                    spdlog::debug("Trident hit to {} of {}!", impactData->damageRootNode->name, ActorTarget->GetName());
                } else {
                    trident->data.proj = proj;
                    spdlog::debug("Trident's target is not an actor.");
                }
                if ((isTargetActor && rtData.weaponDamage > ActorTarget->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth))) {   //  if it will not bounce from the surface or the target gets the lethal hit
                    missileRTD.impactResult = RE::ImpactResult::kImpale;
                    impactData->impactResult = RE::ImpactResult::kImpale;
                    trident->Throw(true);
                    spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
                } else spdlog::debug("{} is bounced from {}!", projBase->GetName(), a_target ? a_target->GetName() : "NULL");
            } else trident->data.position = proj->data.location;
        }
#endif
    }
}

bool ProjectileHook::GetKillOnCollisionArrow(RE::ArrowProjectile* a_this)
{
    bool result = _GetKillOnCollisionArrow(a_this);
    if (WeaponIdentify::IsRelic(a_this, Kratos::Relic::kMjolnir))
        if (Mjolnir::GetSingleton()->data.isPenetrating)
            result = false;
    return result;
}

/*
*   weaponDraw, beginWeaponDraw, WeapEquip_OutMoving, WeapEquip_Out
*/
void PlayerHook::OnEquipItem(RE::PlayerCharacter* a_this, bool a_playAnim)
{
    _OnEquipItem(a_this, !SkipAnim(a_this, a_playAnim));
}
bool PlayerHook::SkipAnim(RE::PlayerCharacter* a_this, bool a_playAnim)
{
    WeaponIdentify::WeaponCheck();
    return !a_playAnim;
}

void PlayerHook::Update(RE::PlayerCharacter* a_this, const float a_delta)
{
    Kratos::GetSingleton()->Update(a_this, a_delta);
    _Update(a_this, a_delta);
}
EventChecker PlayerHook::ProcessEventPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_dispatcher)
{
    ModEvent(a_event, a_dispatcher);
    return _ProcessEventPC(a_sink, a_event, a_dispatcher);
}

bool PlayerHook::ModEvent(RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_dispatcher)
{
    bool ret = false;
    if (a_event && a_dispatcher) {
        std::string eventTag = a_event->tag.data();
        switch (hash(eventTag.data(), eventTag.size())) {
        case "InterruptCast"_h:
            spdlog::warn("InterruptCast!!");
            break;
        case "CatchLevi"_h:
            spdlog::warn("CatchLevi!!");
            break;
        }
    } return ret;
}

bool AttackHook::ProcessButton(RE::AttackBlockHandler* a_handler, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data)
{
    auto playerCamera = RE::PlayerCamera::GetSingleton();
    if (playerCamera && playerCamera->IsInFirstPerson()) {
        // In first person, so we skip processing
    } else if (a_event && a_event->QUserEvent() == "Right Attack/Block") {
        if (auto kratos = Kratos::GetSingleton(); kratos->IsAiming() && kratos->GetEquippedRelic() == Kratos::Relic::kNone) {
            if (WeaponIdentify::isLeviathanAxe && WeaponIdentify::LeviathanAxe) {
                auto AnArchos = RE::PlayerCharacter::GetSingleton();
                auto eqManager = RE::ActorEquipManager::GetSingleton();
                auto Levi = LeviathanAxe::GetSingleton();
                Levi->Call(true);
            //    WeaponIdentify::skipEquipAnim = true;
            //    std::jthread skipEquipAnimThred([=](){
            //        std::this_thread::sleep_for(std::chrono::milliseconds(300));
            //        WeaponIdentify::skipEquipAnim = false;
            //    });
            //    skipEquipAnimThred.detach();
                ObjectUtil::Actor::SkipEquipAnimation(AnArchos);
                eqManager->EquipObject(AnArchos, WeaponIdentify::LeviathanAxe, nullptr, 1U, nullptr, false, false, false, true);
                ObjectUtil::Actor::SkipEquipAnimation(AnArchos, _skipEquipAnim, _load3Ddelay, _skipLoad3D);
                WeaponIdentify::WeaponCheck();
            } else spdlog::info("Levi is not callable");
        }
        spdlog::debug("Attack type is kRight");
    }
    return _ProcessButton(a_handler, a_event, a_data);
}
void AttackHook::OnMeleeHit(RE::Actor* a_target, RE::HitData& a_this)
{
    BeforeDamage(a_target, a_this);
    _OnMeleeHit(a_target, a_this);
}

inline void AttackHook::BeforeDamage(RE::Actor* a_target, RE::HitData& a_this)
{
    if (a_target && a_this.aggressor.get().get()) {
        if (auto kratos = Kratos::GetSingleton()) {
            if (a_target->IsPlayerRef()) {
                if (kratos->IsInRage(a_target)) {
                    if (kratos->GetLastTriggeredRageType() == Kratos::Rage::kFury) {
                        kratos->RestoreRage(a_target, kratos->CalcRageDamageOrBuffAmount(-a_this.totalDamage));
                        a_this.totalDamage = 0.f;
                    } else if (kratos->GetLastTriggeredRageType() == Kratos::Rage::kValor) {
                        kratos->_gettingHittedInValor = true;
                        a_this.totalDamage = 0.f;
                    }
                } else if (Config::BuffRageWhenDamaged) {kratos->RestoreRage(a_target, *kratos->values.rageBuffAmount);}
            } else if (auto agressor = a_this.aggressor.get().get(); agressor && agressor->IsPlayerRef() && !a_target->IsDead()) {
                if (kratos->IsInRage(agressor)) {
                    if (!(a_this.flags & RE::HitData::Flag::kBash))
                        a_this.totalDamage = a_this.totalDamage + a_this.totalDamage * (Config::BarehandedDamageMult - 1.f);
              //      a_this.totalDamage = Config::BarehandedDamage / (RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty);
                    if (agressor->AsActorValueOwner())
                        agressor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, a_this.totalDamage);
                    return;
              //      kratos->RestoreRage(agressor, kratos->CalcRageDamageOrBuffAmount(a_this.totalDamage));
                }
                else if (WeaponIdentify::isBarehanded) {
                    if (!(a_this.flags & RE::HitData::Flag::kBash))
                        a_this.totalDamage = a_this.totalDamage + a_this.totalDamage * (Config::BarehandedDamageMult - 1.f);
              //      a_this.totalDamage = Config::BarehandedDamage / (RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty);
                    kratos->RestoreRage(agressor, kratos->CalcRageDamageOrBuffAmount(a_this.totalDamage, 2.f));
                } else kratos->RestoreRage(agressor, kratos->CalcRageDamageOrBuffAmount(a_this.totalDamage));
            }
        }
    }
}
inline void AttackHook::BeforeDamage(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
    if (a_this && a_AllCdPointCollector) {
        auto& rtData = a_this->GetProjectileRuntimeData();
        for (auto& point : a_AllCdPointCollector->hits) {
        //    const auto proj     = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableA);
            const auto target   = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

            if (target && target->formType == RE::FormType::ActorCharacter) {
                if (const auto victim = target->As<RE::Actor>(); victim && victim->IsPlayerRef()) {
                    if (auto kratos = Kratos::GetSingleton(); kratos->IsInRage(victim)) {
                        if (kratos->GetLastTriggeredRageType() == Kratos::Rage::kFury) {
                            kratos->RestoreRage(victim, kratos->CalcRageDamageOrBuffAmount(-rtData.weaponDamage));
                            rtData.weaponDamage = 0.f;
                        } else if (kratos->GetLastTriggeredRageType() == Kratos::Rage::kValor) {
                            kratos->_gettingHittedInValor = true;
                            rtData.weaponDamage = 0.f;
                        }
                    }
                }
            }
        }
    }
}