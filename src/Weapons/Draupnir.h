#pragma once
#include "util.h"
#include "settings.h"

class Draupnir
{
public:
    static Draupnir* GetSingleton() {static Draupnir singleton; return &singleton;}
    bool Initialize();

    enum class TrailColor : std::uint32_t {
        kWhite = 0u,
        kIceBlue = 1u,
        kSkyBlue = 2u,
        kBlue = 3u,
        kYellow = 4u,
        kGold = 5u,
        kSilver = 6u
    };
    struct TrailData {
        TrailOverride trailOverride;
        TrailTransformOverride transformOverride = TrailTransformOverride();

        RE::NiColorA GetColorByIndex(const uint32_t a_index);
        TrailData(const std::string a_meshDirectory = Config::TrailModelPathDef, const float a_intensity = 1.f) {
            trailOverride = TrailOverride(
                Config::TrailLifetimeMult * a_intensity, 
                GetColorByIndex(Config::TrailColorIndexDraupnir), 
                Config::TrailColorScaleMult * a_intensity, 
                a_meshDirectory
            );
        }
    };

    struct Data {
        RE::NiPointer<RE::Projectile>   proj;
        RE::NiPointer<RE::NiAVObject>   model;
        RE::TESObjectWEAP*              weap    = nullptr;
        RE::EnchantmentItem*            ench    = nullptr;
        RE::AlchemyItem*                poison  = nullptr;
        RE::NiPointer<RE::NiNode>       weaponModelCopy;
        RE::NiPointer<RE::NiNode>       replacedProjectileModel;
        float damage                    = 0.f;
        float gravity                   = 2.69f;
        float rotationSpeed             = Config::ThrowRotationSpeed; //  rad/s
        float throwingChargeDuration    = 0.f;
    };

    static inline Data data;
    static inline TrailData trailData;

    static inline RE::NiPointer<RE::Projectile>   DraupnirSpearProjectiles[9];                            //  they will be explode after Draupnir's Call move
    static inline RE::NiPointer<RE::Projectile>   DraupnirsCallProjectiles[9];                            //  hard to explain
    static inline RE::NiPointer<RE::NiNode>       DraupnirSpearHitBones[9];                               //  keeps attached nodes of throwed draupnirs for forced detonation*
    static inline RE::NiPointer<RE::Actor>        DraupnirSpearHitActors[9];                              //  keeps hitten actors
    static inline RE::NiPointer<RE::Projectile>   DraupnirSpearProjectileL;

    static inline RE::SpellItem*        SpellDraupnirProjL          = nullptr;
    static inline RE::SpellItem*        SpellDraupnirsCallProjL     = nullptr;
    static inline RE::BGSProjectile*    DraupnirSpearProjBaseL      = nullptr;
    static inline RE::BGSProjectile*    DraupnirsCallProjBaseL      = nullptr;

    static inline RE::BGSExplosion* StuckedDraupnir     = nullptr;
    static inline RE::BGSExplosion* DraupnirExplosion   = nullptr;
    static inline RE::BGSArtObject* DraupnirStuckedFX   = nullptr;

    static void Update(const float a_delta);
    static void Throw();
    static void MeleeThrow();
    static void ArtilleryOfTheAncients(const float a_delay, const float a_duration);
    static void RainOfDraupnir();
    static void AddSpearHit(RE::NiNode* a_bone, RE::Actor* a_actor = nullptr);
    static void AddSpearHit(RE::Projectile* a_proj);
    static void Call(const float a_damage, const float a_force);
    static void StartExplosions(const float a_delay);
    static void SetExplosionMagnitude(const float a_magnitude) {explosionMagnitude = a_magnitude;}
    static void StartChargingThrow(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    static void ReplaceStickedProjectileModel(RE::Projectile* a_proj);

    /* forced detonation?
    forced detonation needed for living targets, because timing projectile explosions not working after hitting to actors.
    */
    static inline AsyncUtil::GameTime projectileUpdate;
    static inline AsyncUtil::GameTime trailUpdate;
private:
friend class ProjectileHook;
    static inline float explosionMagnitude = 1.f;
    static inline float nextExplosionTime = 0.f;
    static inline float timeToDoneExplosions = 0.f;
    static inline float explosionDelay = 0.f;
    static inline size_t currentHitIndex = 0;
    static inline bool explosionsStarted = false;
    static inline std::vector<RE::FormID> MeleeHitProjectileIDs;
    static inline RE::BSFixedString DefaultDraupnirModel;
    static inline std::vector<std::tuple<RE::NiPointer<RE::NiNode>, RE::NiPointer<RE::Actor>, RE::NiPointer<RE::Projectile>>> spearHits;
    static void TriggerExplosions(float a_delay, float a_force, RE::ProjectileHandle* a_pHandle);
    static void TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::ProjectileHandle* a_pHandle, RE::Actor* a_target);
    static void TriggerExplosionAtLocation(RE::Projectile* a_proj, RE::ProjectileHandle* a_pHandle);

    static inline float nextLaunchTime = 0.0f;
    static inline float nextLaunchDelay = 0.0f;
    static inline float lastLaunchTime = 0.0f;
    static inline bool draupnirRainStarted = false;
};
