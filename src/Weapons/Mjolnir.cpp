

/*
void SmartRelicWeapon::PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) 
{
    if (runtimeData.projectile && a_impactData) {
        auto missileProjectile = runtimeData.projectile->As<RE::MissileProjectile>();
        const bool isTargetActor = a_target ? a_target->formType == RE::FormType::ActorCharacter : false;
        const auto victim = isTargetActor ? a_target->As<RE::Actor>() : nullptr;
        auto& rtData = missileProjectile->GetProjectileRuntimeData();
        auto& missileRTD = missileProjectile->GetMissileRuntimeData();
        auto shooter = rtData.shooter ? rtData.shooter.get()->As<RE::Actor>() : nullptr;
        const auto projBase = missileProjectile->GetProjectileBase();

        if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kLeviathanAxe)) {
            trailUpdate.Done();
            bool stopSounds = true;
            if (isTargetActor)  {runtimeData.lastHitActors.insert(runtimeData.lastHitActors.begin(), victim->GetHandle()); if (runtimeData.lastHitActors.size() > 4) runtimeData.lastHitActors.pop_back();}
            else if (a_target)  {runtimeData.lastHitForms.insert(runtimeData.lastHitForms.begin(), a_target); if (runtimeData.lastHitForms.size() > 4) runtimeData.lastHitForms.pop_back();}
            if (IsArriving()) {
                spdlog::debug("levi impacted to {} while arriving", a_target ? a_target->GetName() : "NULL");
                auto arrivingState = GetCurrentState() ? dynamic_cast<ArrivingState*>(GetCurrentState()) : nullptr;
                if (arrivingState && victim && victim == arrivingState->GetNextTarget())  Levi->arrivingLevi.targets.erase(Levi->arrivingLevi.targets.begin());
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                a_impactData->impactResult = RE::ImpactResult::kBounce;
                Levi->Call(false, true);
            } else if (runtimeData.impactResult == RE::ImpactResult::kBounce) {
                if (isTargetActor) {
                    runtimeData.projState = ProjectileState::kHavok;
                    for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                    missileRTD.impactResult = RE::ImpactResult::kBounce;
                    a_impactData->impactResult = RE::ImpactResult::kBounce;
                }
            }

            const bool itWillStick = a_impactData->impactResult == RE::ImpactResult::kStick;
            if (const bool isHoming = IsHoming()) {
                if (isHoming && isTargetActor && victim == Levi->homingLevi.GetNextTarget()) Levi->homingLevi.targets.erase(Levi->homingLevi.targets.begin());
                runtimeData.projState = ProjectileState::kNone;
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kDestroy;
                missileRTD.impactResult = RE::ImpactResult::kDestroy;
                a_impactData->impactResult = RE::ImpactResult::kDestroy;
                Levi->Throw(false, true, isHoming);
                stopSounds = false;
                spdlog::debug("{} is hit to {} ({:8x}) and passed through!", projBase->GetName(), a_target ? a_target->GetName() : "NULL",  a_target ? a_target->formID : 0x0);
            } else if (itWillStick) {
        //        if (isTargetActor && rtData.weaponDamage > (victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) / 10.f)) {
        //            ObjectUtil::Actor::PushActorAway(victim, projBase->data.force, rtData.linearVelocity);
        //        }
                runtimeData.stuckedActor.reset(victim);
                runtimeData.stuckedBone.reset(a_impactData->damageRootNode);
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kStick;
                a_impactData->impactResult = RE::ImpactResult::kStick;
                missileRTD.impactResult = RE::ImpactResult::kStick;
                runtimeData.projState = ProjectileState::kStucked;
                spdlog::debug("{} is sticked to {}!", projBase->GetName(), runtimeData.stuckedBone ? runtimeData.stuckedBone->name : "NULL");
            } else {
                for (auto& impact : rtData.impacts) impact->impactResult = RE::ImpactResult::kBounce;
                a_impactData->impactResult = RE::ImpactResult::kBounce;
                missileRTD.impactResult = RE::ImpactResult::kBounce;
                runtimeData.projState = ProjectileState::kHavok;
                spdlog::debug("{} is bounced from {}!", projBase->GetName(), runtimeData.stuckedBone ? runtimeData.stuckedBone->name : "NULL");
            }
            if (stopSounds)
                Levi->soundData.FadeThrowingLoopSounds(369);
        }
        else if (WeaponIdentify::IsRelic(projBase, Kratos::Relic::kMjolnir)) {
            auto mjolnir = Mjolnir::GetSingleton();
            mjolnir->trailUpdate.Done();
            bool skipIt = false;
            bool stopSounds = true;
            if (isTargetActor)  {mjolnir->data.lastHitActors.insert(mjolnir->data.lastHitActors.begin(), victim); if (mjolnir->data.lastHitActors.size() > 4) mjolnir->data.lastHitActors.pop_back();}
            else if (a_target)  {mjolnir->data.lastHitForms.insert(mjolnir->data.lastHitForms.begin(), a_target); if (mjolnir->data.lastHitForms.size() > 4) mjolnir->data.lastHitForms.pop_back();}
            else skipIt = true;
            if (projBase == mjolnir->MjolnirProjBaseA) {
                spdlog::debug("mjolnir impacted to {} while arriving", a_target ? a_target->GetName() : "NULL");
                if (isTargetActor && victim == mjolnir->arrivingMjolnir.GetNextTarget()) mjolnir->arrivingMjolnir.targets.erase(mjolnir->arrivingMjolnir.targets.begin());
                missileRTD.impactResult = RE::ImpactResult::kDestroy;
                a_impactData->impactResult = RE::ImpactResult::kDestroy;
                mjolnir->Call(false, true);
            } else {
            //    if (isTargetActor && rtData.weaponDamage > (victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth) / 20.f))
            //        ObjectUtil::Actor::PushActorAway(victim, projBase->data.force);
            //    const bool itWillStick = (a_impactData->impactResult == RE::ImpactResult::kStick || missileRTD.impactResult == RE::ImpactResult::kStick);
                bool itWillBounce = (a_impactData->impactResult == RE::ImpactResult::kBounce || missileRTD.impactResult == RE::ImpactResult::kBounce);
                const bool isHoming = IsHoming();
                if (skipIt && !isHoming) {
                    mjolnir->data.projState = Mjolnir::ProjectileState::kHavok;
                    missileRTD.impactResult = RE::ImpactResult::kBounce;
                    a_impactData->impactResult = RE::ImpactResult::kBounce;
                    itWillBounce = true;
                    spdlog::debug("mjolnir hit an unidentified object and bounced!");
                } else if (!itWillBounce || isTargetActor || isHoming) {
                    if (isHoming && isTargetActor && victim == mjolnir->homingMjolnir.GetNextTarget()) mjolnir->homingMjolnir.targets.erase(mjolnir->homingMjolnir.targets.begin());
                    mjolnir->data.projState = Mjolnir::ProjectileState::kNone;
                    missileRTD.impactResult = RE::ImpactResult::kDestroy;
                    a_impactData->impactResult = RE::ImpactResult::kDestroy;
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
    }
}

*/