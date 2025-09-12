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
void ProjectileHook::GetLinearVelocityArrow(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
{
    _GetLinearVelocityArrow(a_this, a_outVelocity);

    //  this is required because this function is calling two times in one frame
    if (AsyncUtil::GameTime::GetEngineTime() - lastUpdate < (*g_deltaTimeRealTime / 10.f)) {
    //    spdlog::trace("duplication cancelled");
        return;
    } else {
        lastUpdate = AsyncUtil::GameTime::GetEngineTime();
    //    spdlog::trace("delta t = {}, {}", *g_deltaTimeRealTime, *g_engineTime);
        if (ThrowCallMutex.try_lock()) {
            LeviAndDraupnir(a_this);
            ThrowCallMutex.unlock();
        }
    }
}
void ProjectileHook::LeviAndDraupnir(RE::Projectile* a_this)
{
    auto& runtimeData = a_this->GetProjectileRuntimeData();
    const auto& shooter = runtimeData.shooter;

    if (shooter.native_handle() == 0x100000)    // player only, 0x100000 = player
    {
        auto projectileNode = a_this->Get3D2();
        if (!projectileNode) {spdlog::warn("projectile's 3d not loaded"); return;}
        const auto projBase = a_this->GetProjectileBase();
        const auto& livingTime = runtimeData.livingTime;
        RE::Actor* AnArchos = shooter.get().get() ? shooter.get().get()->As<RE::Actor>() : nullptr;

        if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kLeviathanAxe)) {
            auto kratos = Kratos::GetSingleton();
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

            Levi->data.model = projectileNode;
        //    for (auto& projTrail : Levi->data.projTrails) {
        //        if (projTrail) {
        //            projTrail->lifetime = projTrail->age + 0.1;
        //        }
        //    }

        //    auto animatedBone = projectileNode->GetObjectByName("Cylinder02");
        //    auto leviModel = Levi->data.weaponModel;//(WeaponIdentify::LeviathanAxe->AsReference1() ? WeaponIdentify::LeviathanAxe->AsReference1()->Get3D() : nullptr);
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
                    float speed = Config::ThrowSpeed;
                //  speed *= (*g_deltaTimeRealTime / *g_deltaTime); //  for not be effected by slow motion/fast motion
                    vel = linearDir * speed;
                //  skip gravity effect for a while
                    if (livingTime < Config::NoGravityDurationLeviathan) {
                        projBase->data.gravity = 0.1f;
                    } else {
                        projBase->data.gravity = Levi->data.gravity;
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

                    RE::NiPoint3 rotation = linearDir;
#ifdef EXPERIMENTAL_LEVIATHAN_MODEL
/*
leviAngle is a variable that keeps euler rotations of the leviathan's projectile.
Levi->data.yAngle is the tilt angle of the leviathan.
livingTime is the living time of the projectile from launching.
Levi->data.rotationSpeed is the rotation speed of the projectile.

the spin must look like the leviathan axe's throwed state spin from the god of war 2018 and ragnarok games.
*/
                    float spinAngle = livingTime * Levi->data.rotationSpeed;
                    float yTilt = Levi->data.yAngle;

                //  quaternion method
                    RE::NiQuaternion yTiltQuat = 
                        MathUtil::Algebra::QuaternionWithAngleAxis(yTilt, RE::NiPoint3(frontVec));

                    RE::NiQuaternion spinQuat = 
                        MathUtil::Algebra::QuaternionWithAngleAxis(spinAngle, RE::NiPoint3(upVec));

                    RE::NiQuaternion finalQuat = MathUtil::Algebra::MultiplyQuaternions(yTiltQuat, spinQuat);

                    RE::NiMatrix3 finalRot = MathUtil::Algebra::QuaternionToMatrix(finalQuat);

                //    finalRot.ToEulerAnglesXYZ(rotation);

                //  matrix method
                    RE::NiMatrix3 tiltY;
                    tiltY.SetEulerAnglesXYZ(0.f, Levi->data.yAngle, 0.f);

                    RE::NiMatrix3 spinZ;
                    spinZ.SetEulerAnglesXYZ(0.f, 0.f, spinAngle);

                    RE::NiMatrix3 rotationMatrix = tiltY * spinZ;
                //    rotationMatrix.ToEulerAnglesXYZ(rotation);

                //  rodrigues method
                    RE::NiPoint3 localRight(linearDir.UnitCross(RE::NiPoint3(upVec)));
                    RE::NiPoint3 localUp(linearDir.UnitCross(localRight));
                    RE::NiPoint3 rotatingAxis = MathUtil::Algebra::RotateVectorRodrigues(localUp, linearDir, Levi->data.yAngle);
                    rotatingAxis.Unitize();
                    rotation = MathUtil::Algebra::RotateVectorRodrigues(linearDir, rotatingAxis, livingTime * Levi->data.rotationSpeed);
#endif
                    leviAngle.x = asin(rotation.z);
                //    leviAngle.y = yTilt;
                    leviAngle.z = atan2(rotation.x, rotation.y);
                }
            } else {
                if (projBase != Levi->LeviProjBaseA) {
#ifdef PRECISION
                    if (APIs::precision || APIs::Request()) {
                        APIs::precision->RemoveProjectileCollision(AnArchos->GetHandle(), Levi->collisionDefinition);
                    }
#endif
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
                Levi->arrivingLevi.linearDistance = distance;
                auto linearArrivingDir = handPos - leviPos;
                linearArrivingDir.Unitize();
                Levi->arrivingLevi.linearArrivingDir = linearArrivingDir;
                Levi->arrivingLevi.currentDir = linearDir;

            //  speed calculation
#ifdef NEW_ARRIVING_METHOD
                float arrSpeed = distance / arrivingTime;//Levi->data.arrivingRoute.second / arrivingTime;
#else
                float arrSpeed = distance / arrivingTime;
#endif

            //  set speed limits
                const bool isCatchable = (distance <= Config::CatchingTreshold) || (distance <= (*g_deltaTime * vel.Length()));
                if (arrSpeed < Config::MinArrivalSpeed && !isCatchable) arrSpeed = Config::MinArrivalSpeed;
                else if (arrSpeed > Config::MaxArrivalSpeed)            arrSpeed = Config::MaxArrivalSpeed;

                if (isCatchable) {
                    if (Levi->GetThrowState() == tState::kArriving) Levi->SetThrowState(tState::kArrived);
                    Levi->Catch();
                    spdlog::debug("Levi proj catched");
                }
#ifdef SMART_ARRIVING_METHOD
                if (Levi->IsArriving(a_this)) {
                    auto& aLevi = Levi->arrivingLevi;
                    float aLivingTime = aLevi.GetLivingTime();
                    const float t = std::clamp(aLivingTime / aLevi.timeToArrive, 0.f, 1.f);
                    RE::NiMatrix3 handRot   = targetPoint->world.rotate;
                    RE::NiPoint3 palmDir    = handRot * RE::NiPoint3(backVec);
                    RE::NiPoint3 handForward= handRot * RE::NiPoint3(upVec);
                    palmDir.Unitize();
                    handForward.Unitize();

                    const float handSideOffsetMult = 0.3f;//MathUtil::Algebra::ParabolicClamp(aLevi.arrivingRelativeAngleZ, 0.f, 0.3f);

                    RE::NiPoint3 p0 = aLevi.startPosition;
                    RE::NiPoint3 p3 = handPos;
                    RE::NiPoint3 p1 = p0 + linearArrivingDir * aLevi.linearDistanceFromStart / 3.f;
                    RE::NiPoint3 p2 = p3 + palmDir * (aLevi.linearDistanceFromLastCallPos / 3.f + 20.f) + handForward * (aLevi.linearDistanceFromLastCallPos * handSideOffsetMult + 10.f);

                    RE::NiPoint3 cubicBezierPos = MathUtil::Algebra::BezierPoint(t, p0, p1, p2, p3);
                    RE::NiPoint3 bezierDir = cubicBezierPos - leviPos;
                    bezierDir.Unitize();
                    aLevi.desiredDir = bezierDir;

                //    aLevi.speed = MathUtil::Algebra::ParabolicClamp(t, Config::MinArrivalSpeed, Config::MinArrivalSpeed + (arrSpeed - Config::MinArrivalSpeed) * 1.5f);
                    aLevi.speed = aLevi.speed < arrSpeed ? aLevi.speed : arrSpeed;
                    vel = MathUtil::Angle::BlendVectors(Levi->data.projState == LeviathanAxe::ProjectileState::kLaunched ? Levi->data.lastVelocity : (aLevi.linearArrivingDir * aLevi.speed), aLevi.desiredDir * aLevi.speed, aLivingTime / 0.2f);
                //    vel = aLevi.desiredDir * (aLevi.speed < arrSpeed ? aLevi.speed : arrSpeed);
                    float height = leviPos.z - AnArchos->GetPosition().z;

                    RE::NiPoint3 rotation = handRot * RE::NiPoint3(backVec);

                    RE::NiPoint3 localRight(linearDir.UnitCross(RE::NiPoint3(upVec)));
                    RE::NiPoint3 localUp(linearDir.UnitCross(localRight));
                    RE::NiPoint3 rotatingAxis = MathUtil::Algebra::RotateVectorRodrigues(localUp, linearDir, Levi->data.yAngle);
                    rotatingAxis.Unitize();
                    rotation = MathUtil::Algebra::RotateVectorRodrigues(linearDir, rotatingAxis, livingTime * Levi->data.rotationSpeed);

                //    leviAngle.x = asin(rotation.z);
                //    leviAngle.z = atan2(rotation.x, rotation.y);

                //    if (t < 0.9f/*aLevi.timeToArrive > 0.1f*/) {
                //        if (auto aTarget = aLevi.GetNextTarget(leviPos); aTarget) {
                //            auto targetPos = aTarget->GetPosition() + (aTarget->GetBoundMax() + aTarget->GetBoundMin()) * 0.75f;
                //            auto targetDir = (targetPos - leviPos);
                //            targetDir.Unitize();
                //            targetDir *= aLevi.speed;
                //            height = leviPos.z - aTarget->GetPosition().z;
                //            vel = targetDir;//MathUtil::Angle::BlendVectors(vel, targetDir, (t - 0.2f));
                //        }
                //    } else vel = aLevi.linearArrivingDir * aLevi.speed;
                //    spdlog::debug("Levi proj arriving, t = {}, speed = {}, distance = {}",
                //        t, vel.Length(), distance);
                //    float dampFactor = std::clamp((height - 10.f) / 90.f, 0.f, 1.f);
                //    if (vel.z < 0.f) vel.z *= dampFactor;  //  damp vertical speed
                }
#elifdef NEW_ARRIVING_METHOD
                RE::NiMatrix3 handRot   = targetPoint->world.rotate;
                RE::NiPoint3 palmDir    = handRot * RE::NiPoint3(backVec);
                RE::NiPoint3 handForward= handRot * RE::NiPoint3(upVec);
                palmDir.Unitize();
                handForward.Unitize();

            //    std::vector<float> targets = {0.f, 0.25f, 0.5f, 0.75f, 1.f};
            //    auto spineNode = AnArchos->GetNodeByName("NPC Spine2 [Spn2]");
            //    float attractedRelativeAngleZ = TWO_PI * MathUtil::Algebra::AttractToNearest(Levi->data.arrivingRelativeAngleZ, targets, 1.f);
            //    RE::NiPoint3 mainAxisDirection = MathUtil::Algebra::PitchYawToVector(-90.f, 180.f);
            //    mainAxisDirection.Unitize();

                RE::Actor* target = Levi->arrivingLevi.GetNextTarget(leviPos);
                RE::NiPoint3 targetPos;
                if (target) {
                    targetPos = target->GetPosition() + (target->GetBoundMax() + target->GetBoundMin()) * 0.75f;
                }
                RE::NiPoint3 p0 = leviPos;
                RE::NiPoint3 p3 = handPos;
                RE::NiPoint3 p1 = p0 + linearArrivingDir * distance / 3.f;
                RE::NiPoint3 p2 = p3 + palmDir * (Levi->arrivingLevi.linearDistanceFromLastCallPos / 3.f + 20.f) + handForward * (Levi->arrivingLevi.linearDistanceFromStart / 4.f + 20.f);

                float charZ = AnArchos->GetPosition().z;
                if (p2.z < charZ) p2.z = charZ;

                if (target) {
                    p1 = p2 = p3 = targetPos;
                }

                const float t = std::clamp(livingTime / Config::ArrivalTime, 0.f, 1.f);
                RE::NiPoint3 cubicBezierPos = MathUtil::Algebra::BezierPoint(t, p0, p1, p2, p3);//Levi->data.arrivingRoute.first[(int)(t * (Levi->data.arrivingRoute.first.size() - 1))];

                RE::NiPoint3 bezierDir = cubicBezierPos - leviPos;
                bezierDir.Unitize();
                vel = bezierDir * MathUtil::Algebra::ParabolicClamp(t * 0.9f, Config::MinArrivalSpeed, Config::MinArrivalSpeed + (arrSpeed - Config::MinArrivalSpeed) * 1.5f);
#else
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
                RE::NiPoint3 crossDir(direction.UnitCross(downVec));
                vel = (direction * arrSpeed) + (crossDir * offset);
#endif
                RE::NiPoint3 curvyDir = vel;
                curvyDir.Unitize();
                leviAngle.x = asin(curvyDir.z);
                leviAngle.z = atan2(curvyDir.x, curvyDir.y);

                if (auto spineNode = AnArchos->GetNodeByName("NPC Spine2 [Spn2]"); spineNode && !isCatchable && distance > 100.f) {
                    auto spineForwardDir = spineNode->world.rotate * RE::NiPoint3(frontVec);
                    spineForwardDir.z = 0.f;  //  ignore vertical direction
                    spineForwardDir.Unitize();

                    RE::NiPoint3 linearDir2D(linearArrivingDir.x, linearArrivingDir.y, 0.f);
                    float dot = spineForwardDir.Dot(linearDir2D);
                    float det = spineForwardDir.x * linearDir2D.y - spineForwardDir.y * linearDir2D.x;
                    auto& arrivingRelativeAngle = Levi->arrivingLevi.arrivingRelativeAngleZ;
                    arrivingRelativeAngle = atan2(det, dot);  //  angle between spine forward direction and axe direction
                    arrivingRelativeAngle = MathUtil::Angle::NormalAbsoluteAngle(arrivingRelativeAngle);  //  normalize angle to [0, PI]
                    arrivingRelativeAngle = MathUtil::Angle::RadianToDegree(arrivingRelativeAngle) / 360.f;  //  normalize angle to [0, 1]
                    std::vector<float> targets = {0.f, 0.25f, 0.5f, 0.75f, 1.f};
                    float snapStrength = Config::ArrivalAngleSnap;
                //    if ((arrivingRelativeAngle < 0.124f || arrivingRelativeAngle > 0.876f) && snapStrength < 0.8f) snapStrength += 0.1f;
                    arrivingRelativeAngle = MathUtil::Algebra::AttractToNearest(arrivingRelativeAngle, targets, snapStrength);    //  for helping to the blender generator 
                //    float previousAngle; AnArchos->GetGraphVariableFloat("fArrivingWeaponDirection", previousAngle);
                //    spdlog::debug("arrivingRelativeAngle: {}, previousAngle: {}", arrivingRelativeAngle, previousAngle);
                //    float maxDelta = PI2 * (*g_deltaTime);
                //    float delta = arrivingRelativeAngle - previousAngle;
                //    delta = delta < 0.6f ? std::clamp(delta, -maxDelta, maxDelta) : delta;
                //    arrivingRelativeAngle = previousAngle + delta;
                    AnArchos->SetGraphVariableFloat("fArrivingWeaponDirection", arrivingRelativeAngle);
                }
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
            auto kratos = Kratos::GetSingleton();
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
                        projBase->data.gravity = 0.1f;
                    } else {
                        projBase->data.gravity = mjolnir->data.gravity;
                    }
                    //apply rotation
                    float rot = Config::ThrowRotationSpeed * livingTime;
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

                if (mjolnir->isMjolnirArriving) {
                    if (projBase != mjolnir->MjolnirProjBaseA) {
                        runtimeData.flags |= (1 << 25); 
                    //  spdlog::debug("[HOOK] mjolnir destroyed before call");
                        return;
                    }
                    if (mjolnir->MjolnirProjectileA != a_this) mjolnir->MjolnirProjectileA = a_this;
                    if (mjolnir->GetThrowState() == tStateM::kCanArrive) mjolnir->SetThrowState(tStateM::kArriving);

#ifdef SMART_ARRIVING_METHOD
                    if (mjolnir->IsArriving(a_this)) {
                        auto& aMjolnir = mjolnir->arrivingMjolnir;
                        aMjolnir.lastVelocity = vel;
                        float aLivingTime = aMjolnir.GetLivingTime();
                        auto aTarget = aMjolnir.GetNextTarget(mjolnirPos);
                        const float blendTime = 0.f;
                        const float t = std::clamp(aLivingTime / (aMjolnir.timeToArrive + blendTime), 0.f, 1.f);
                        RE::NiMatrix3 handRot   = targetPoint->world.rotate;
                        RE::NiPoint3 palmDir    = handRot * RE::NiPoint3(backVec);
                        RE::NiPoint3 handForward= handRot * RE::NiPoint3(upVec);
                        palmDir.Unitize();
                        handForward.Unitize();

                        const float handSideOffsetMult = 0.3f;//MathUtil::Algebra::ParabolicClamp(aMjolnir.arrivingRelativeAngleZ, 0.f, 0.3f);

                        RE::NiPoint3 p0 = mjolnirPos;
                        RE::NiPoint3 p3 = handPos;
                        RE::NiPoint3 p1 = p0 + linearArrivingDir * aMjolnir.linearDistanceFromStart / 3.f;
                        RE::NiPoint3 p2 = p3 + palmDir * (aMjolnir.linearDistanceFromLastCallPos / 3.f + 20.f) + handForward * (aMjolnir.linearDistanceFromLastCallPos * handSideOffsetMult + 10.f);

                        float speed = MathUtil::Algebra::ParabolicClamp(t, Config::MinArrivalSpeed, Config::MinArrivalSpeed + (aMjolnir.speed - Config::MinArrivalSpeed) * 1.5f);
                        RE::NiPoint3 cubicBezierPos = MathUtil::Algebra::BezierPoint(t, p0, p1, p2, p3);
                        RE::NiPoint3 bezierDir = cubicBezierPos - mjolnirPos;
                    //    uint16_t tOffset = 0;
                    //    if (t == 1) {
                    //        cubicBezierPos = handPos;
                    //        bezierDir = cubicBezierPos - mjolnirPos;
                    //    } else {
                    //        while (tOffset <= aMjolnir.routeResolution) {
                    //            cubicBezierPos = aMjolnir.arrivingRoute.first[aMjolnir.routeResolution * t + tOffset];//MathUtil::Algebra::BezierPoint(t, p0, p1, p2, p3);
                    //            bezierDir = cubicBezierPos - mjolnirPos;
                    //            const float resolutionDistance = bezierDir.Length();
                    //            if ((speed * *g_deltaTime) > resolutionDistance) {
                    //                spdlog::debug("arriving speed is higher than resolution! {},  t: {}", resolutionDistance / *g_deltaTime, t);
                    //                tOffset++;
                    //            }
                    //            else {
                    //                spdlog::debug("arriving speed is fine t: {}, speed {}", t, speed);
                    //                break;
                    //            }
                    //        }
                    //    }
                        bezierDir.Unitize();
                        aMjolnir.desiredDir = bezierDir;
                        aMjolnir.desiredVelocity = bezierDir * aMjolnir.speed;

                        vel = MathUtil::Angle::BlendVectors(aMjolnir.startVelocity, aMjolnir.desiredVelocity, 1.f /*aLivingTime / blendTime*/);
                    //    vel = aMjolnir.desiredDir * (aMjolnir.speed < arrSpeed ? aMjolnir.speed : arrSpeed);
                        float height = mjolnirPos.z - AnArchos->GetPosition().z;
                    //    RE::NiPoint3 targetVelocity; AnArchos->GetLinearVelocity(targetVelocity);
                    //    vel += targetVelocity;

                    //    if (aTarget) {
                    //        auto targetPos = aTarget->GetPosition() + (aTarget->GetBoundMax() + aTarget->GetBoundMin()) * 0.75f;
                    //        auto targetDir = (targetPos - mjolnirPos);
                    //        targetDir.Unitize();
                    //        targetDir *= aMjolnir.speed;
                    //        height = mjolnirPos.z - aTarget->GetPosition().z;
                    //        vel = MathUtil::Angle::BlendVectors(vel, targetDir, (t - 0.2f));
                    //    }
                    //    float dampFactor = std::clamp((height - 10.f) / 90.f, 0.f, 1.f);
                    //    if (vel.z < 0.f) vel.z *= dampFactor;  //  damp vertical speed
                    }
#else
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
#endif
                    RE::NiPoint3 curvyDir = vel;
                    curvyDir.Unitize();

                    //apply rotation
                    mjolnirAngle = mjolnir->data.lastEulerAngles;
                    mjolnirAngle.z = atan2(curvyDir.x, curvyDir.y);
                    mjolnir->data.lastEulerAngles = mjolnirAngle;
                } else if (Config::MjolnirArrivingDelay.has_value()) {  // blending state
                    auto& aMjolnir = mjolnir->arrivingMjolnir;
                    const float blendTime = *Config::MjolnirArrivingDelay;
                    auto velocityWhenCalled = mjolnir->data.projState == Mjolnir::ProjectileState::kLaunched ? mjolnir->data.lastVelocity : (aMjolnir.linearArrivingDir * aMjolnir.speed);
                    vel = MathUtil::Angle::BlendVectors(velocityWhenCalled, linearArrivingDir * mjolnir->arrivingMjolnir.speed, mjolnir->arrivingMjolnir.GetLivingTime() / blendTime);
                    aMjolnir.startVelocity = vel;
                    RE::NiPoint3 curvyDir = vel;
                    curvyDir.Unitize();
                    mjolnirAngle = mjolnir->data.lastEulerAngles;  //  keep last angle
                    mjolnirAngle.x = asin(curvyDir.z);
                    mjolnirAngle.z = atan2(curvyDir.x, curvyDir.y);
                    mjolnir->data.lastEulerAngles = mjolnirAngle;
                }
                if (auto spineNode = AnArchos->GetNodeByName("NPC Spine2 [Spn2]"); spineNode && !isCatchable && distance > 100.f) {
                    auto spineForwardDir = spineNode->world.rotate * RE::NiPoint3(frontVec);
                    spineForwardDir.z = 0.f;  //  ignore vertical direction
                    spineForwardDir.Unitize();

                    RE::NiPoint3 linearDir2D(linearArrivingDir.x, linearArrivingDir.y, 0.f);
                    float dot = spineForwardDir.Dot(linearDir2D);
                    float det = spineForwardDir.x * linearDir2D.y - spineForwardDir.y * linearDir2D.x;
                    auto& arrivingRelativeAngle = mjolnir->arrivingMjolnir.arrivingRelativeAngleZ;
                    arrivingRelativeAngle = atan2(det, dot);  //  angle between spine forward direction and axe direction
                    arrivingRelativeAngle = MathUtil::Angle::NormalAbsoluteAngle(arrivingRelativeAngle);  //  normalize angle to [0, PI]
                    arrivingRelativeAngle = MathUtil::Angle::RadianToDegree(arrivingRelativeAngle) / 360.f;  //  normalize angle to [0, 1]
                    std::vector<float> targets = {0.f, 0.25f, 0.5f, 0.75f, 1.f};
                    float snapStrength = Config::ArrivalAngleSnap;
                //    if ((arrivingRelativeAngle < 0.124f || arrivingRelativeAngle > 0.876f) && snapStrength < 0.8f) snapStrength += 0.1f;
                    arrivingRelativeAngle = MathUtil::Algebra::AttractToNearest(arrivingRelativeAngle, targets, snapStrength);    //  for helping to the blender generator
                //    float previousAngle; AnArchos->GetGraphVariableFloat("fArrivingWeaponDirection", previousAngle);
                //    float maxDelta = PI2 * (*g_deltaTime);
                //    float delta = arrivingRelativeAngle - previousAngle;
                //    delta = delta < 0.6f ? std::clamp(delta, -maxDelta, maxDelta) : delta;
                //    arrivingRelativeAngle = previousAngle + delta;
                    AnArchos->SetGraphVariableFloat("fArrivingWeaponDirection", arrivingRelativeAngle);
                }
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
                } else {spdlog::warn("WEIRD, target or levi is not exists"); return true;}
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
            if (/*rtData.livingTime < 0.01f || */(mjolnir->isMjolnirCalled && Config::DontDamageWhileArrive)) return true;

            for (auto& point : a_AllCdPointCollector->hits) {
                const auto target   = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

                const bool isHoming = mjolnir->IsHoming(a_this);// && (mjolnir->homingMjolnir.GetNextTarget() || mjolnir->homingMjolnir.GetLivingTime() < 1.f);
                const bool isArriving = projBase == mjolnir->MjolnirProjBaseA;
                bool isSameTarget = false;
                bool isTargetActor = target ? target->formType == RE::FormType::ActorCharacter : false;
                const auto victim = target ? target->As<RE::Actor>() : nullptr;
                if (target && a_this != target->AsProjectile()) {
#ifdef EXPERIMENTAL_PROJECTILE_HIT_EXPLOSION
                    auto weaponIE = ObjectUtil::Actor::GetInventoryEntryDataForBoundObject(shooter, mjolnir->data.weap);
                    RE::HitData hitData; hitData.Populate(shooter, nullptr, weaponIE);
                    RE::hkVector4 point(a_this->data.location);
                    RE::TESHavokUtilities::AddExplosionImpulse(a_this->Get3D2(), point, 1000.f, &hitData);
                    spdlog::debug("!mjolnir caused explosion impulse to target: {} by {} damage", target->GetName(), hitData.totalDamage);
                    return true;
#endif

                    if (!isTargetActor && !mjolnir->data.lastHitForms.empty() && std::find(mjolnir->data.lastHitForms.begin(), mjolnir->data.lastHitForms.end(), target) != mjolnir->data.lastHitForms.end()) isSameTarget = true;
                    else if (victim && victim != shooter) {
                        if (a_this->IsMissileProjectile()) {
                            if (!mjolnir->data.lastHitActors.empty() && std::find(mjolnir->data.lastHitActors.begin(), mjolnir->data.lastHitActors.end(), victim) != mjolnir->data.lastHitActors.end()) isSameTarget = true;
                            if (isHoming && victim == mjolnir->homingMjolnir.GetNextTarget() && isSameTarget && rtData.livingTime > 0.5f) isSameTarget = false;
                            if (isArriving && victim == mjolnir->arrivingMjolnir.GetNextTarget() && isSameTarget && rtData.livingTime > 0.5f) isSameTarget = false;
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
                } else {spdlog::warn("WEIRD, target or mjolnir is not exists"); return true;}
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

RE::Projectile::ImpactData* ProjectileHook::GetArrowImpactData(RE::ArrowProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a3, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, char a7)
{
    auto impactData = _GetArrowImpactData(proj, a_target, a3, a_velocity, a_collidable, a6, a7);
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
                spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
            } else if (itWillStick) {
        //        if (isTargetActor && rtData.weaponDamage > (ActorTarget->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) / 10.f)/**/) {
        //            ObjectUtil::Actor::PushActorAway(ActorTarget, projBase->data.force, rtData.linearVelocity);
        //        }
                Levi->data.stuckedActor = ActorTarget;
                Levi->data.stuckedBone = impactData->damageRootNode;
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

            if (shooter && (APIs::precision || APIs::Request())) {
                APIs::precision->RemoveProjectileCollision(shooter->GetHandle(), Levi->collisionDefinition);
            }
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
                    spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
                } else {
                    mjolnir->data.projState = Mjolnir::ProjectileState::kHavok;
                    spdlog::debug("{} is bounced from {}!", projBase->GetName(), a_target ? a_target->GetName() : "NULL");
                }
            }
        }
        else if (projBase == Draupnir::DraupnirsCallProjBaseL) {
            missileRTD.impactResult = RE::ImpactResult::kDestroy;
            impactData->impactResult = RE::ImpactResult::kDestroy;
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
    if (a_event && a_event->QUserEvent() == "Right Attack/Block") {
        if (auto kratos = Kratos::GetSingleton(); kratos->IsAiming() && kratos->GetEquippedRelic() == Kratos::Relic::kNone) {
            if (WeaponIdentify::isLeviathanAxe && WeaponIdentify::LeviathanAxe) {
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