#include "hook.h"
#include "util.h"
#include "settings.h"
#include "MainKratosCombat.h"
using namespace Util;
using tState = LeviathanAxe::ThrowState;
using tStateM = Mjolnir::ThrowState;

static std::mutex ThrowCallMutex;

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
void ProjectileHook::GetLinearVelocityArrow(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
{
    _GetLinearVelocityArrow(a_this, a_outVelocity);
    if (ThrowCallMutex.try_lock()) {
        LeviAndDraupnir(a_this);
        ThrowCallMutex.unlock();
    }
}
void ProjectileHook::LeviAndDraupnir(RE::Projectile* a_this)
{
    auto& runtimeData = a_this->GetProjectileRuntimeData();
    const auto& shooter = runtimeData.shooter;

    if (shooter.native_handle() == 0x100000)    // player only, 0x100000 == player
    {
        auto projectileNode = a_this->Get3D2();
        if (!projectileNode) {spdlog::warn("projectile's 3d not loaded"); return;}
        const auto projBase = a_this->GetProjectileBase();
        const auto& livingTime = runtimeData.livingTime;
        RE::Actor* AnArchos = shooter.get().get() ? shooter.get().get()->As<RE::Actor>() : nullptr;

        if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kLeviathanAxe)) {
        //  auto animatedBone = projectileNode->GetObjectByName("Slash");
        //  auto leviModel = (WeaponIdentify::LeviathanAxe->AsReference1() ? WeaponIdentify::LeviathanAxe->AsReference1()->Get3D() : nullptr);
        //  if (animatedBone && leviModel) {
        //      auto animatedNode = animatedBone->AsNode();
        //      auto leviNode = leviModel->AsNode();
        //      if (animatedNode && leviNode) {
        //          animatedNode->AttachChild(leviNode, false);
        //          spdlog::debug("levi model changed!");
        //      } else spdlog::debug("animated node or levinode null");
        //  } else spdlog::debug("animated bone or leviModel null");
            //identify levi
        //  spdlog::debug("Levi proj flags: {:032b}", runtimeData.flags);   // "10000100001101010000000101010000" kFading, kUnk7, kChainShatter, 
        //  if (runtimeData.flags & (1 << 15)) {
        //      runtimeData.flags &= ~(1 << 15);        //remove the RE::Projectile::Flags::kDestroyAfterHit flag
        //  //  a_this->inGameFormFlags &= (1 << 10);   //RE::TESForm::RecordFlags::kPersistent;
        //      spdlog::debug("Levi proj kDestroyAfterHit flag removed");
        //  }

            //location
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

            if (livingTime > 0.3f && Levi->GetThrowState() == tState::kThrown) Levi->SetThrowState(tState::kCanArrive);

            //speed
            auto& vel = runtimeData.linearVelocity;

            auto linearDir = vel;
            linearDir.Unitize();

            Levi->data.position = leviPos;
    //        if (APIs::Request()){// && livingTime < *g_deltaTimeRealTime) {
    //        //    auto rigidBody = NifUtil::Collision::GetRigidBody(projectileNode);
    //        //    RE::hkpRigidBody* hkpRigidBody = rigidBody ? static_cast<RE::hkpRigidBody*>(rigidBody->referencedObject.get()) : nullptr;
    //        //    hkpRigidBody->userData;
    //        //    APIs::precision->ApplyHitImpulse2(AnArchos->GetHandle(), AnArchos->GetHandle(), hkpRigidBody, vel, handPos, 10.f);
    //            APIs::precision->AddAttackTrail(projectileNode->AsNode(), AnArchos->GetHandle(), AnArchos->parentCell, a_this, PRECISION_API::TrailOverride());
    //        }
            if (!Levi->isAxeCalled) {
                Levi->data.throwedTime = livingTime;

                if (Levi->IsHoming(a_this)) {
                    auto& hLevi = Levi->homingLevi;
                    float hLivingTime = hLevi.GetLivingTime();
                    auto hTarget = hLevi.GetNextTarget(leviPos);
                    if (hLivingTime > 1.2f && livingTime > 0.2f && hTarget) {
                        auto targetPos = hTarget->GetPosition() + (hTarget->GetBoundMax() + hTarget->GetBoundMin()) / 2.f;
                        auto targetDir = (targetPos - leviPos);
                        targetDir.Unitize();
                        const float speed = hLevi.speed * (livingTime < 1.f ? (livingTime + 0.2f) : 1.2f);
                        vel = MathUtil::Angle::BlendVectors(linearDir, targetDir, ((livingTime - 0.3f) / 3.f), true) * speed;
                    } else {
                        if (hLivingTime > 1.8f && livingTime > 0.3f && hLevi.targets.empty()) Levi->Call(false, AnArchos);

                        // waving effect
                        float waveX = hLevi.waveAmplitude * cos(hLevi.waveFrequency * hLivingTime);
                        float waveZ = hLevi.waveAmplitude * sin(hLevi.waveFrequency * hLivingTime);

                        auto targetPos = AnArchos->GetPosition() + (AnArchos->GetBoundMax() + AnArchos->GetBoundMin()) / 2.f;
                        float distance = targetPos.GetDistance(leviPos);
                        auto targetDir = targetPos - leviPos;
                        targetDir.Unitize();
                        RE::NiPoint3 circularVel;
                        RE::NiPoint3 originVelocity; AnArchos->GetLinearVelocity(originVelocity);
                        circularVel.x = -hLevi.speed * targetDir.y + originVelocity.x + waveX;
                        circularVel.y = hLevi.speed * targetDir.x + originVelocity.y;
                        circularVel.z = targetDir.z + originVelocity.z + waveZ;

                        circularVel += targetDir * (distance - 100.f) / 0.2f;

                        vel = MathUtil::Angle::BlendVectors(vel, circularVel, ((livingTime - 0.2f) / 3.f));
                    }

                    RE::NiPoint3 curvyDir = vel;
                    curvyDir.Unitize();
                    leviAngle.x = asin(curvyDir.z);
                    leviAngle.z = atan2(curvyDir.x, curvyDir.y);
                } else {
                    float speed = Config::ThrowSpeed;
                //  speed *= (*g_deltaTimeRealTime / *g_deltaTime); //  for not be effected by slow motion/fast motion
                    vel = linearDir * speed;
                //  skip gravity effect for a while
                    RE::NiPoint3 gravity(0.f, 0.f, (1.f + a_this->GetGravity() * projBase->data.gravity));
                    if (livingTime < Config::NoGravityDurationLeviathan) {
                        vel += (gravity);
                    }
                    //apply rotation
                //  leviAngle.x = asin(linearDir.z);
                //  leviAngle.z -= atan2(linearDir.x, linearDir.y) + rot / 30.f;    //
                //  leviAngle.z -= rot / 30.f;                      //main rotation
    //  matrix things
    //              SetRotationMatrix(projectileNode->local.rotate, -linearDir.x, linearDir.y, linearDir.z);    //  sa * cb, ca * cb, sa
    //              NiPoint3 vectorLeviX = {projectileNode->local.rotate.entry[0][0],   //  cosa
    //                                      projectileNode->local.rotate.entry[1][0],   //  sina
    //                                      projectileNode->local.rotate.entry[2][0]};  //  0.0
    //              NiPoint3 vectorLeviY = {projectileNode->local.rotate.entry[0][1],   // -sina * cosb
    //          /*forward vector of levi*/  projectileNode->local.rotate.entry[1][1],   //  cosa * cosb
    //                                      projectileNode->local.rotate.entry[2][1]};  //  sinb
    //              NiPoint3 vectorLeviZ = {projectileNode->local.rotate.entry[0][2],   //  sina * sinb
    //          /*upward vector of levi*/   projectileNode->local.rotate.entry[1][2],   // -cosa * sinb
    //                                      projectileNode->local.rotate.entry[2][2]};  //  cosb
    
                //  float rot = Config::ThrowRotationSpeed * livingTime;
                //  float yAngle = Levi->data.yAngle;       //0.35f = 20 degrees for lateral throw, PI/2 = 90 degrees for vertical throw
                    leviAngle.x = asin(linearDir.z);
                //  leviAngle.y = yAngle;
                //  leviAngle.z = rot;
                //  RE::NiMatrix3 copyMatrix = projectileNode->local.rotate;                                    //  copy of original rotation matrix
                //  MathUtil::Algebra::SetRotationMatrix(copyMatrix, -linearDir.x, linearDir.y, linearDir.z);   //  set rotation of copied matrix
                //  MathUtil::Algebra::RotateMatrixAroundAxisses(copyMatrix, yAngle, 0.f, rot);                 //  add rotation
                //  projectileNode->local.rotate = copyMatrix;                                                  //  apply rotation
                //  projectileNode->world.rotate = copyMatrix;
                }
            } else  {
                if (projBase != Levi->LeviProjBaseA) {
                    runtimeData.flags |= (1 << 25); 
                //  spdlog::debug("[HOOK] levi destroyed before call"); 
                    return;
                }
                if (Levi->LeviathanAxeProjectileA != a_this) Levi->LeviathanAxeProjectileA = a_this;
                if (Levi->GetThrowState() == tState::kCanArrive) Levi->SetThrowState(tState::kArriving);

            //  float passedArrTime = livingTime - Levi->throwedTime;
            //  if (passedArrTime < 0.f) passedArrTime = livingTime;
                float arrivingTime = Config::ArrivalTime - livingTime;//passedArrTime;
                if (arrivingTime < *g_deltaTime * 2.f) arrivingTime = *g_deltaTime * 2.f;

                const float distance = handPos.GetDistance(leviPos);

            //  speed calculation
                float arrSpeed = distance / arrivingTime;

            //  set speed limits
                const bool isCatchable = distance < Config::CatchingTreshold || distance <= (*g_deltaTime * arrSpeed);
                if (arrSpeed < Config::MinArrivalSpeed && !isCatchable) arrSpeed = Config::MinArrivalSpeed;
                else if (arrSpeed > Config::MaxArrivalSpeed)            arrSpeed = Config::MaxArrivalSpeed;

                if (isCatchable) {
                    if (Levi->GetThrowState() == tState::kArriving) Levi->SetThrowState(tState::kArrived);
                    Levi->Catch();
                    spdlog::debug("Levi proj catched");
                }
            //  if (distance <= (*g_deltaTime * arrSpeed)) {
            //      if (distance >= Config::CatchingTreshold) {
            //          if (tState == tState::kArriving) Levi->SetThrowState(tState::kArrived);
//
            //          if (!(runtimeData.flags & (1 << 25))) {
            //              Levi->Catch();
            //              spdlog::debug("Levi proj catched");
            //          }
            //      }
            //      runtimeData.flags |= (1 << 25);
            //  //  spdlog::debug("levi is catched!! dist {}, speed {}, dt {}, dtRT {}", distance, arrSpeed, *g_deltaTime, *g_deltaTimeRealTime);
            //  }

                const float sinAngle    = Config::ArrivalRoadCurveMagnitude;//sinf(Config::ArrivalRoadCurveMagnitude * 0.017453292f);
        //      float distMoved = runtimeData.distanceMoved;
        //      float halfWay   = distance - distMoved;
        //      float tertiaWay = 2.f * distance - distMoved;
        //      float quartWay  = 3.f * distance - distMoved;
        //      float sexWay    = 5.f * distance - distMoved;   //  sexWay = 0 means the axe finished 5/6 of the way
        //      if      (sexWay     < 0.f)  {/**/ arrSpeed *= 0.3f; sinAngle = 0.f;}
        //      else if (quartWay   < 0.f)  {/**/ arrSpeed *= 0.7f; sinAngle *= 0.4f;}
        //      else if (tertiaWay  < 0.f)  {/**/ arrSpeed *= 0.8f; sinAngle *= 0.7f;}
        //      else if (halfWay    < 0.f)  {/**/ arrSpeed *= 0.9f; sinAngle *= 0.9f;}              //  turns direction to target while getting close
        //
        //      if  (arrSpeed < Config::MinArrivalSpeed)    arrSpeed = Config::MinArrivalSpeed;

                const float currSpeed = vel.Length() + 1.f;
                const float timeToArrive = distance / currSpeed;
            //  arrSpeed *= (*g_deltaTimeRealTime / *g_deltaTime);      //  for not be effected by slow motion/fast motion *but causing very buggy situations
                float curveMult = timeToArrive / Config::ArrivalTime;   //  for reduce the offset while getting close
                if (curveMult > 1.6f) curveMult = 1.6f;
                const float offset = arrSpeed * sinAngle * curveMult;

                // new direction calculating method
                RE::NiPoint3 direction = (handPos - leviPos);                       //  direction of the axe to player's hand
                direction.Unitize();                                                //  normalize direction
                RE::NiPoint3 crossDir({0.f, 0.f, 0.f});
            //    RE::NiPoint3 playerDir({0.f, 0.f, 0.f});
                crossDir = direction.UnitCross(downVec);
                vel = (direction * arrSpeed) + (crossDir * offset);
            //      spdlog::debug("distance: {}, passed arriving time: {}", distance, passedArrTime);
                RE::NiPoint3 curvyDir = vel;
                curvyDir.Unitize();
                leviAngle.x = asin(curvyDir.z);
                leviAngle.z = atan2(curvyDir.x, curvyDir.y);
            //  MathUtil::Algebra::SetRotationMatrix(projectileNode->local.rotate, -curvyDir.x, curvyDir.y, curvyDir.z);

            //  float xRot  = Config::ArrivalRotationX * livingTime;
            //  float yRot  = Config::ArrivalRotationY * livingTime;
            //  float zRot  = Config::ArrivalRotationZ * livingTime;
            //  float sacb  = curvyDir.x + xRot;
            //  float cacb  = curvyDir.y + yRot;
            //  float sb    = curvyDir.z + zRot;
            //  MathUtil::Algebra::SetRotationMatrix(projectileNode->local.rotate, -sacb, cacb, sb);

                // old direction calculating method
            //  RE::NiPoint3 midPoint = (handPos + leviPos) / 2;
            //  RE::NiPoint3 curvePoint = midPoint + crossDir * offset;
            //  if (offset < 90.f) {
            //      curvePoint = handPos;
            //  }
            //      spdlog::debug("curvepoint dist: {} midpoint dist: {}", curvePoint.GetDistance(handPos), midPoint.GetDistance(handPos));
            //  RE::NiPoint3 curvyDir = (curvePoint - leviPos);
            //  curvyDir.Unitize();
                // rotate
            //  float arrivingAngle = PI2 * passedArrTime / Config::ArrivalTime;
            //  leviAngle.x = asin(curvyDir.z);
            //  leviAngle.z = atan2(curvyDir.x, curvyDir.y);
            //  if (leviAngle.y < PI2) {
            //      leviAngle.y -= 0.0017;
            //  }
            //  if (leviAngle.x < PI) {
            //      leviAngle.x -= 0.00017;
            //  }
            //  if (leviAngle.z < 0.0) {
            //      leviAngle.z += PI;
            //  }
            //  if (direction.x < 0.0) {
            //      leviAngle.z += PI;
            //  }
            //  float rot = Config::ArrivalRotationSpeed * livingTime;
            //  RE::NiMatrix3 copyMatrix = projectileNode->local.rotate;
            //  SetRotationMatrix(copyMatrix, -curvyDir.x, curvyDir.y, curvyDir.z);
            //  MathUtil::Algebra::RotateMatrixAroundAxisses(copyMatrix, -PI2, PI, rot);
            //  projectileNode->local.rotate = copyMatrix;
            //
            //  vel = curvyDir * 1200.f;
            }
        }
        else if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kMjolnir)) {
            //location
            const auto targetPoint = WeaponIdentify::RHandBone;
            if (!targetPoint) {spdlog::warn("can't found your hand node for Mjolnir call!!"); return;}
            auto& handPos   = targetPoint->world.translate;
            auto& mjolnirPos   = a_this->data.location;
            auto& mjolnirAngle = a_this->data.angle;

            auto mjolnir = Mjolnir::GetSingleton();
            if (a_this != mjolnir->LastMjolnirProjectile) mjolnir->LastMjolnirProjectile = a_this;
            mjolnir->MjolnirProjectileT = a_this;
            mjolnir->data.model = a_this->Get3D();

            if (livingTime > 0.3f && mjolnir->GetThrowState() == tStateM::kThrown) mjolnir->SetThrowState(tStateM::kCanArrive);

            //speed
            auto& vel = runtimeData.linearVelocity;
        //    mjolnir->data.lastOrientation = {a_this->GetAngleX(), a_this->GetHeadingAngle(mjolnir->data.position, true)};

            auto linearDir = vel;
            linearDir.Unitize();

            mjolnir->data.position = mjolnirPos;
            mjolnir->data.lastVelocity = vel;
            if (!mjolnir->isMjolnirCalled) {
                mjolnir->data.throwedTime = livingTime;
            //    float speed = vel.Length();
            //    speed *= (*g_deltaTimeRealTime / *g_deltaTime); //  for not be effected by slow motion/fast motion
            //    vel = linearDir * speed;

            //  skip gravity effect for a while

                if (mjolnir->IsHoming(a_this)) {
                    auto& hMjolnir = mjolnir->homingMjolnir;
                    float hLivingTime = hMjolnir.GetLivingTime();
                    auto hTarget = hMjolnir.GetNextTarget(mjolnirPos);
                    if (hLivingTime > 1.2f && livingTime > 0.3f && hTarget) {
                        auto targetPos = hTarget->GetPosition() + (hTarget->GetBoundMax() + hTarget->GetBoundMin()) / 2.f;
                        auto targetDir = (targetPos - mjolnirPos);
                        targetDir.Unitize();
                        const float speed = hMjolnir.speed * (livingTime < 1.f ? (livingTime + 0.2f) : 1.2f);
                        vel = MathUtil::Angle::BlendVectors(linearDir, targetDir, ((livingTime - 0.3f) / 3.f), true) * speed;
                    } else {
                        if (hLivingTime > 1.8f && livingTime > 0.6f && hMjolnir.targets.empty()) mjolnir->Call(false, AnArchos);

                        // waving effect
                        float waveX = hMjolnir.waveAmplitude * cos(hMjolnir.waveFrequency * hLivingTime);
                        float waveZ = hMjolnir.waveAmplitude * sin(hMjolnir.waveFrequency * hLivingTime);

                        auto targetPos = AnArchos->GetPosition() + (AnArchos->GetBoundMax() + AnArchos->GetBoundMin()) / 2.f;;
                        float distance = targetPos.GetDistance(mjolnirPos);
                        auto targetDir = targetPos - mjolnirPos;
                        targetDir.Unitize();
                        RE::NiPoint3 circularVel;
                        RE::NiPoint3 originVelocity; AnArchos->GetLinearVelocity(originVelocity);
                        circularVel.x = -hMjolnir.speed * targetDir.y + originVelocity.x + waveX;
                        circularVel.y = hMjolnir.speed * targetDir.x + originVelocity.y;
                        circularVel.z = targetDir.z + originVelocity.z + waveZ;

                        circularVel += targetDir * (distance - 100.f) / 0.2f;

                        vel = MathUtil::Angle::BlendVectors(vel, circularVel, ((livingTime - 0.3f) / 3.f));
                    }

                    RE::NiPoint3 curvyDir = vel;
                    curvyDir.Unitize();
                    mjolnirAngle.x = asin(curvyDir.z);
                    mjolnirAngle.z = atan2(curvyDir.x, curvyDir.y);
                } else {
                    if (livingTime < Config::NoGravityDurationMjolnir) {
                        RE::NiPoint3 gravity(0.f, 0.f, (1.f + a_this->GetGravity() * projBase->data.gravity));
                        gravity *= (*g_deltaTimeRealTime / *g_deltaTime);
                        vel += (gravity);
                    }
                    //apply rotation
                    float rot = Config::ThrowRotationSpeed * livingTime;
                //  float yAngle = mjolnir->data.yAngle;       //0.35f = 20 degrees for lateral throw, PI/2 = 90 degrees for vertical throw
                    mjolnirAngle.x = asin(linearDir.z);
                //  mjolnirAngle.y = yAngle;
                    mjolnirAngle.y = rot;
                }
            } else {
                if (projBase != mjolnir->MjolnirProjBaseA) {
                    runtimeData.flags |= (1 << 25); 
                //  spdlog::debug("[HOOK] mjolnir destroyed before call"); 
                    return;
                }
                if (mjolnir->MjolnirProjectileA != a_this) mjolnir->MjolnirProjectileA = a_this;
                if (mjolnir->GetThrowState() == tStateM::kCanArrive) mjolnir->SetThrowState(tStateM::kArriving);

            //  float passedArrTime = livingTime - mjolnir->throwedTime;
            //  if (passedArrTime < 0.f) passedArrTime = livingTime;
                float arrivingTime = Config::ArrivalTime - livingTime;//passedArrTime;
                if (arrivingTime < *g_deltaTime * 2.f) arrivingTime = *g_deltaTime * 2.f;

                const float distance = handPos.GetDistance(mjolnirPos);

            //  speed calculation
                float arrSpeed = distance / arrivingTime;

            //  set speed limits
                const bool isCatchable = distance < Config::CatchingTreshold || distance <= (*g_deltaTime * arrSpeed);
                if (arrSpeed < Config::MinArrivalSpeed && !isCatchable) arrSpeed = Config::MinArrivalSpeed * 0.7f;
                else if (arrSpeed > Config::MaxArrivalSpeed)            arrSpeed = Config::MaxArrivalSpeed * 0.7f;

                if (isCatchable) {
                    if (mjolnir->GetThrowState() == tStateM::kArriving) mjolnir->SetThrowState(tStateM::kArrived);
                    mjolnir->Catch();
                    spdlog::debug("Mjolnir proj catched");
            //    } else {
            //        a_this->GetCurrent3D()->world.rotate.SetEulerAnglesXYZ(40,30,60); //  these methods can't change rotation
            //        a_this->Get3D()->world.rotate.SetEulerAnglesXYZ(40,30,60);
            //        a_this->Get3D1(false)->world.rotate.SetEulerAnglesXYZ(40,30,60);
            //        a_this->Get3D2()->world.rotate.SetEulerAnglesXYZ(40,30,60);
                }

                const float sinAngle    = Config::ArrivalRoadCurveMagnitude;//sinf(Config::ArrivalRoadCurveMagnitude * 0.017453292f);

                const float currSpeed = vel.Length() + 1.f;
                const float timeToArrive = distance / currSpeed;
            //  arrSpeed *= (*g_deltaTimeRealTime / *g_deltaTime);      //  for not be effected by slow motion/fast motion *but causing very buggy situations
                float curveMult = timeToArrive / Config::ArrivalTime;   //  for reduce the offset while getting close
                if (curveMult > 1.6f) curveMult = 1.6f;
                const float offset = arrSpeed * sinAngle * curveMult;

                // new direction calculating method
                RE::NiPoint3 direction = (handPos - mjolnirPos);        //  direction of the mjolnir to player's hand
                direction.Unitize();                                    //  normalize direction
                RE::NiPoint3 crossDir({0.f, 0.f, 0.f});
                crossDir = direction.UnitCross(downVec);
                vel = (direction * arrSpeed) + (crossDir * offset);
            //    vel = MathUtil::Angle::BlendVectors(mjolnir->data.lastVelocity, vel, (livingTime / 3.f));
                RE::NiPoint3 curvyDir = vel;
                curvyDir.Unitize();

                //apply rotation
                float rot = Config::ArrivalRotationSpeed * livingTime;
                mjolnirAngle.x = 5*rot;
                mjolnirAngle.z = atan2(curvyDir.x, curvyDir.y);
            }
        }
        else if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kDraupnirSpear)) {
            if (projBase == Draupnir::DraupnirSpearProjBaseL) {
                for (auto ID : Draupnir::MeleeHitProjectileIDs) {
                    if (ID == a_this->formID) {
                        if (livingTime > 0.1f) {
                            runtimeData.flags|= (1 << 25);
                            Draupnir::DraupnirSpearProjBaseL->model = Draupnir::DefaultDraupnirModel;
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
                trident->data.model = a_this->Get3D();
                trident->data.position = a_this->data.location;
            }
        }
#endif
    }
}

void ProjectileHook::GetCollisionArrow(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
    if (LeviAndDraupnirHit(a_this, a_AllCdPointCollector)) return;
    AttackHook::BeforeDamage(a_this, a_AllCdPointCollector);
    _GetCollisionArrow(a_this, a_AllCdPointCollector);
}
void ProjectileHook::GetCollisionMissile(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
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
            Levi->data.model = a_this->Get3D();
            if (Levi->isAxeCalled && Config::DontDamageWhileArrive) return true;

            for (auto& point : a_AllCdPointCollector->hits) {
            //    const auto ourProj  = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableA);
                const auto target   = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

                const bool isHoming = Levi->IsHoming(a_this);// && (mjolnir->homingMjolnir.GetNextTarget() || mjolnir->homingMjolnir.GetLivingTime() < 1.f);
                const bool isArriving = projBase == Levi->LeviProjBaseA;
                bool isSameTarget = false;
                bool isTargetActor = target ? target->formType == RE::FormType::ActorCharacter : false;
            //  if (ourProj) {
            //      if (projBase == Levi->LeviProjBaseL) a_this->data.angle.z = Config::MaxAxeStuckAngle;
            //      else a_this->data.angle.y = Config::MaxAxeStuckAngle;
            //  }
                if (target && a_this != target->AsProjectile()) {
                    if (!isTargetActor && !Levi->data.lastHitForms.empty() && std::find(Levi->data.lastHitForms.begin(), Levi->data.lastHitForms.end(), target) != Levi->data.lastHitForms.end()) isSameTarget = true;
                    else if (const auto victim = target->As<RE::Actor>(); victim) {
                        if (a_this->IsMissileProjectile()) {
                            if (!Levi->data.lastHitActors.empty() && std::find(Levi->data.lastHitActors.begin(), Levi->data.lastHitActors.end(), victim) != Levi->data.lastHitActors.end()) isSameTarget = true;
                            if (isHoming && victim == Levi->homingLevi.GetNextTarget() && isSameTarget && rtData.livingTime > 0.5f) isSameTarget = false;
                            if (!victim->IsDead()) {
                                if (!isArriving) Levi->data.stuckedActor = victim;

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
                }
                if (isSameTarget || (!isTargetActor && (isArriving || isHoming))) {
                    return true;
                } else {
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
        //    mjolnir->data.model = a_this->Get3D();
            if (mjolnir->isMjolnirCalled && Config::DontDamageWhileArrive) return true;

            for (auto& point : a_AllCdPointCollector->hits) {
                const auto target   = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

                const bool isHoming = mjolnir->IsHoming(a_this);// && (mjolnir->homingMjolnir.GetNextTarget() || mjolnir->homingMjolnir.GetLivingTime() < 1.f);
                const bool isArriving = projBase == mjolnir->MjolnirProjBaseA;
                bool isSameTarget = false;
                bool isTargetActor = target ? target->formType == RE::FormType::ActorCharacter : false;
                if (target && a_this != target->AsProjectile()) {
                    if (!isTargetActor && !mjolnir->data.lastHitForms.empty() && std::find(mjolnir->data.lastHitForms.begin(), mjolnir->data.lastHitForms.end(), target) != mjolnir->data.lastHitForms.end()) isSameTarget = true;
                    else if (const auto victim = target->As<RE::Actor>(); victim) {
                        if (a_this->IsMissileProjectile()) {
                            if (!mjolnir->data.lastHitActors.empty() && std::find(mjolnir->data.lastHitActors.begin(), mjolnir->data.lastHitActors.end(), victim) != mjolnir->data.lastHitActors.end()) isSameTarget = true;
                            if (isHoming && victim == mjolnir->homingMjolnir.GetNextTarget() && isSameTarget && rtData.livingTime > 0.5f) isSameTarget = false;
                            if (!victim->IsDead()) {
                                if (mjolnir->data.weap && mjolnir->data.ench && mjolnir->data.ench->effects[0])
                                    ObjectUtil::Enchantment::ChargeInventoryWeapon(shooter, mjolnir->data.weap, -mjolnir->data.ench->effects[0]->effectItem.magnitude);

                                if (shooter && !isArriving) {
                                    shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);

                                    auto kratos = Kratos::GetSingleton();
                                    kratos->RestoreRage(shooter, kratos->CalcRageDamageOrBuffAmount(rtData.weaponDamage, 0.5f));
                                }
                            }
                        }
                    }
                } else spdlog::warn("WEIRD, target or mjolnir is not exists");
                if (isSameTarget || (!isTargetActor && (isArriving || isHoming))) return true;
            }

            if (mjolnir->GetThrowState() == tStateM::kThrown) mjolnir->SetThrowState(tStateM::kCanArrive);
        }

        else if (projBase == Draupnir::DraupnirSpearProjBaseL) {
            for (auto& point : a_AllCdPointCollector->hits) {
                const auto target = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

                if (target) {
                    if (target->formType == RE::FormType::ActorCharacter) {
                        if (const auto victim = target->As<RE::Actor>(); victim) {
                            if (!victim->IsDead()) {
                                RE::EnchantmentItem* ench = nullptr;
                                if (rtData.weaponSource && rtData.weaponSource->formEnchanting) ench = rtData.weaponSource->formEnchanting;

                                if (shooter) {
                                    if (ench) {
                                        if (const auto eff = ench->GetCostliestEffectItem(); eff && eff->IsHostile()) {
                                            shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, ench);
                                        //  ench->data.chargeOverride -= 1000u;
                                        }
                                    } else {
                                        if (rtData.weaponSource)
                                            shooter->UseSkill(RE::ActorValue::kArchery, 1.8f, rtData.weaponSource);
                                    }
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
                if (target && a_this != target->AsProjectile()) {
                    if (!isTargetActor && !trident->data.lastHitForms.empty() && std::find(trident->data.lastHitForms.begin(), trident->data.lastHitForms.end(), target) != trident->data.lastHitForms.end()) isSameTarget = true;
                    else if (auto victim = target->As<RE::Actor>(); victim) {
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

RE::Projectile::ImpactData* ProjectileHook::GetArrowImpactData(RE::ArrowProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a3, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, char a7)
{
    auto impactData = _GetArrowImpactData(proj, a_target, a3, a_velocity, a_collidable, a6, a7);
    if (proj && impactData) {
        const bool isTargetActor = a_target ? a_target->formType == RE::FormType::ActorCharacter : false;
        const auto ActorTarget = isTargetActor ? a_target->As<RE::Actor>() : nullptr;
        auto& rtData = proj->GetProjectileRuntimeData();
        auto& missileRTD = proj->GetMissileRuntimeData();
        const auto projBase = proj->GetProjectileBase();
        auto Levi = LeviathanAxe::GetSingleton();
        auto mjolnir = Mjolnir::GetSingleton();
#ifdef TRIDENT
        auto trident = Trident::GetSingleton();
#endif

        if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kLeviathanAxe)) {
            if (isTargetActor)  {Levi->data.lastHitActors.insert(Levi->data.lastHitActors.begin(), ActorTarget); if (Levi->data.lastHitActors.size() > 3) Levi->data.lastHitActors.pop_back();}
            else if (a_target)  {Levi->data.lastHitForms.insert(Levi->data.lastHitForms.begin(), a_target); if (Levi->data.lastHitForms.size() > 3) Levi->data.lastHitForms.pop_back();}
            if (projBase == Levi->LeviProjBaseA) {
                spdlog::debug("levi impacted to {} while arriving", a_target ? a_target->GetName() : "NULL");
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                impactData->impactResult = RE::ImpactResult::kBounce;
                Levi->Call();
            } else if (projBase == Levi->LeviProjBaseL) {
                if (isTargetActor) {
                    missileRTD.impactResult = RE::ImpactResult::kBounce;
                    impactData->impactResult = RE::ImpactResult::kBounce;
                }
            }

            const bool itWillStick = (impactData->impactResult == RE::ImpactResult::kStick || missileRTD.impactResult == RE::ImpactResult::kStick);
            if (const bool isHoming = Levi->IsHoming(proj); isHoming) {
                if (isHoming && isTargetActor && ActorTarget == Levi->homingLevi.GetNextTarget()) Levi->homingLevi.targets.erase(Levi->homingLevi.targets.begin());
                missileRTD.impactResult = RE::ImpactResult::kDestroy;
                impactData->impactResult = RE::ImpactResult::kDestroy;
                Levi->Throw(false, true, isHoming);
                spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
            } else if (itWillStick) {
        //        if (isTargetActor && rtData.weaponDamage > (ActorTarget->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) / 10.f)/**/) {
        //            ObjectUtil::Actor::PushActorAway(ActorTarget, projBase->data.force, rtData.linearVelocity);
        //        }
                Levi->data.stuckedActor = ActorTarget;
                Levi->data.stuckedBone = impactData->damageRootNode;
                Levi->isAxeStucked = true;
                spdlog::debug("{} is sticked to {}!", projBase->GetName(), Levi->data.stuckedBone ? Levi->data.stuckedBone->name : "NULL");
            } else spdlog::debug("{} is bounced from {}!", projBase->GetName(), Levi->data.stuckedBone ? Levi->data.stuckedBone->name : "NULL");

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
            if (isTargetActor)  {mjolnir->data.lastHitActors.insert(mjolnir->data.lastHitActors.begin(), ActorTarget); if (mjolnir->data.lastHitActors.size() > 3) mjolnir->data.lastHitActors.pop_back();}
            else if (a_target)  {mjolnir->data.lastHitForms.insert(mjolnir->data.lastHitForms.begin(), a_target); if (mjolnir->data.lastHitForms.size() > 3) mjolnir->data.lastHitForms.pop_back();}
            if (projBase == mjolnir->MjolnirProjBaseA) {
                missileRTD.impactResult = RE::ImpactResult::kDestroy;
                impactData->impactResult = RE::ImpactResult::kDestroy;
                spdlog::debug("mjolnir impacted to {} while arriving", a_target ? a_target->GetName() : "NULL");
                mjolnir->Call();
            } else {
                auto hitOrientation = mjolnir->data.lastVelocity;//a_velocity ? *a_velocity : rtData.linearVelocity;
                hitOrientation.Unitize();
            //    if (isTargetActor && rtData.weaponDamage > (ActorTarget->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) / 20.f))
            //        ObjectUtil::Actor::PushActorAway(ActorTarget, projBase->data.force);
            //    const bool itWillStick = (impactData->impactResult == RE::ImpactResult::kStick || missileRTD.impactResult == RE::ImpactResult::kStick);
                const bool itWillBounce = (impactData->impactResult == RE::ImpactResult::kBounce || missileRTD.impactResult == RE::ImpactResult::kBounce);
                const bool isHoming = mjolnir->IsHoming(proj);
                if (!itWillBounce || isTargetActor || isHoming) {
                    if (isHoming && isTargetActor && ActorTarget == mjolnir->homingMjolnir.GetNextTarget()) mjolnir->homingMjolnir.targets.erase(mjolnir->homingMjolnir.targets.begin());
                    missileRTD.impactResult = RE::ImpactResult::kDestroy;
                    impactData->impactResult = RE::ImpactResult::kDestroy;
                    mjolnir->Throw(true, false, isHoming);
                    spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
                } else spdlog::debug("{} is bounced from {}!", projBase->GetName(), a_target ? a_target->GetName() : "NULL");
            }
        }
        else if (projBase == Draupnir::DraupnirsCallProjBaseL) {
        //    missileRTD.impactResult = RE::ImpactResult::kImpale;
        //    impactData->impactResult = RE::ImpactResult::kImpale;
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
            missileRTD.impactResult = RE::ImpactResult::kStick;
            impactData->impactResult = RE::ImpactResult::kStick;
#ifdef NEW_DRAUPNIR_EXPLOSION_METHOD
            if (ActorTarget && impactData->damageRootNode) {
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
    return impactData;
}
bool ProjectileHook::GetKillOnCollisionArrow(RE::ArrowProjectile* a_this)
{
    return !_GetKillOnCollisionArrow(a_this);
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
    if (a_this) {
        WeaponIdentify::WeaponCheck();

    //    if (auto kratos = Kratos::GetSingleton(); !a_playAnim && !WeaponIdentify::skipEquipAnim && WeaponIdentify::isLeviathanAxe && kratos->IsCanCallAxe()) {
    //        a_this->SetGraphVariableBool("SkipEquipAnimation", _skipEquipAnim);
    //        return false;
    //    }
    }
    return !a_playAnim;
}

void PlayerHook::Update(RE::PlayerCharacter* a_this, const float a_delta)
{
    Kratos::GetSingleton()->Update(a_this);
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
    if (a_event && a_event->QUserEvent() == "Right Attack/Block") {
        if (auto kratos = Kratos::GetSingleton(); kratos->IsAiming() && kratos->GetEquippedRelic() == Kratos::Relic::kNone) {
            if (WeaponIdentify::LeviathanAxe) {
                auto AnArchos = PlayerCharacter::GetSingleton();
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
                        a_this.totalDamage *= Config::BarehandedDamageMult;
              //      a_this.totalDamage = Config::BarehandedDamage / (RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty);
                    if (agressor->AsActorValueOwner())
                        agressor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, a_this.totalDamage);
                    return;
              //      kratos->RestoreRage(agressor, kratos->CalcRageDamageOrBuffAmount(a_this.totalDamage));
                }
                else if (WeaponIdentify::isBarehanded) {
                    if (!(a_this.flags & RE::HitData::Flag::kBash))
                        a_this.totalDamage *= Config::BarehandedDamageMult;
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