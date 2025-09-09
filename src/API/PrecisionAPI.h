#pragma once

/*
* For modders: Copy this file into your own project if you wish to use this API
*/
namespace PRECISION_API
{
	constexpr const auto PrecisionPluginName = "Precision";

	// Available Precision interface versions
	enum class InterfaceVersion : uint8_t
	{
		V1,
		V2,
		V3,
		V4
	};

	// Error types that may be returned by Precision
	enum class APIResult : uint8_t
	{
		// Your API call was successful
		OK,

		// A callback from this plugin has already been registered
		AlreadyRegistered,

		// A callback from this plugin has not been registered
		NotRegistered,
	};

	struct PreHitModifier
	{
		enum class ModifierType : uint8_t
		{
			Damage,
			Stagger
		};

		enum class ModifierOperation : uint8_t
		{
			Additive,
			Multiplicative
		};

		ModifierType modifierType;
		ModifierOperation modifierOperation;
		float modifierValue;
	};

	struct PreHitCallbackReturn
	{
		// if set to true, the hit will be ignored, no matter what. Do this if you need the game to ignore a hit that would otherwise happen (e.g. a parry)
		bool bIgnoreHit = false;

		// modifiers to the hit
		std::vector<PreHitModifier> modifiers;
	};

	struct WeaponCollisionCallbackReturn
	{
		// if set to true, the hit to the weapon owner will be ignored. Otherwise the game will handle the hit normally as if the weapon was the actor's body.
		bool bIgnoreHit = true;
	};

	enum class PrecisionLayerType : uint8_t
	{
		None,
		Attack,  // The layer used for attack collisions
		Body     // The layer used by the cloned body collisions
	};

	struct PrecisionLayerSetupCallbackReturn
	{
		PrecisionLayerSetupCallbackReturn() :
			precisionLayerType(PrecisionLayerType::None), layersToAdd(0), layersToRemove(0)
		{}

		// which layer to alter
		PrecisionLayerType precisionLayerType;

		// the layer bits to add
		uint64_t layersToAdd;

		// the layer bits to remove
		uint64_t layersToRemove;
	};

	struct PrecisionHitData
	{
		PrecisionHitData(RE::Actor* a_attacker, RE::TESObjectREFR* a_target, RE::hkpRigidBody* a_hitRigidBody, RE::hkpRigidBody* a_hittingRigidBody, const RE::NiPoint3& a_hitPos,
			const RE::NiPoint3& a_separatingNormal, const RE::NiPoint3& a_hitPointVelocity, RE::hkpShapeKey a_hitBodyShapeKey, RE::hkpShapeKey a_hittingBodyShapeKey) :
			attacker(a_attacker),
			target(a_target), hitRigidBody(a_hitRigidBody), hittingRigidBody(a_hittingRigidBody), hitPos(a_hitPos), separatingNormal(a_separatingNormal),
			hitPointVelocity(a_hitPointVelocity), hitBodyShapeKey(a_hitBodyShapeKey), hittingBodyShapeKey(a_hittingBodyShapeKey)
		{}

		RE::Actor* attacker;
		RE::TESObjectREFR* target;
		RE::hkpRigidBody* hitRigidBody;
		RE::hkpRigidBody* hittingRigidBody;

		RE::NiPoint3 hitPos;
		RE::NiPoint3 separatingNormal;
		RE::NiPoint3 hitPointVelocity;

		RE::hkpShapeKey hitBodyShapeKey;
		RE::hkpShapeKey hittingBodyShapeKey;
	};

	struct TrailOverride
	{
		TrailOverride() = default;

		TrailOverride(std::optional<float> a_lifetimeMult,
			std::optional<RE::NiColorA> a_baseColorOverride,
			std::optional<float> a_baseColorScaleMult,
			std::optional<std::string> a_meshOverride) :
			lifetimeMult(a_lifetimeMult),
			baseColorOverride(a_baseColorOverride), baseColorScaleMult(a_baseColorScaleMult), meshOverride(a_meshOverride)
		{}

		std::optional<float> lifetimeMult;
		std::optional<RE::NiColorA> baseColorOverride;
		std::optional<float> baseColorScaleMult;
		std::optional<std::string> meshOverride;
	};

	struct CollisionDefinition
	{
		CollisionDefinition() = default;
	
		CollisionDefinition(std::string_view a_nodeName,
			std::optional<uint8_t> a_ID = std::nullopt,
			bool a_bNoRecoil = false,
			bool a_bNoTrail = false,
			bool a_bTrailUseTrueLength = false,
			bool a_bWeaponTip = false,
			float a_damageMult = 1.f,
			std::optional<float> a_duration = std::nullopt,
			std::optional<float> a_durationMult = std::nullopt,
			std::optional<float> a_delay = std::nullopt,
			std::optional<float> a_capsuleRadius = std::nullopt,
			std::optional<float> a_radiusMult = std::nullopt,
			std::optional<float> a_capsuleLength = std::nullopt,
			std::optional<float> a_lengthMult = std::nullopt,
			std::optional<RE::NiTransform> a_transform = std::nullopt,
			std::optional<RE::NiPoint3> a_groundShake = std::nullopt,
			std::optional<TrailOverride> a_trailOverride = std::nullopt) :
			nodeName(a_nodeName),
			ID(a_ID), bNoRecoil(a_bNoRecoil), bNoTrail(a_bNoTrail), bTrailUseTrueLength(a_bTrailUseTrueLength), bWeaponTip(a_bWeaponTip), damageMult(a_damageMult), duration(a_duration), durationMult(a_durationMult), delay(a_delay), capsuleRadius(a_capsuleRadius), radiusMult(a_radiusMult), capsuleLength(a_capsuleLength), lengthMult(a_lengthMult), transform(a_transform), groundShake(a_groundShake), trailOverride(a_trailOverride)
		{}
	
		std::string nodeName;
		std::optional<uint8_t> ID;
		bool bNoRecoil = false;
		bool bNoTrail = false;
		bool bTrailUseTrueLength = false;
		bool bWeaponTip = false;
		float damageMult = 1.f;
		std::optional<float> duration;
		std::optional<float> durationMult;
		std::optional<float> delay;
		std::optional<float> capsuleRadius;
		std::optional<float> radiusMult;
		std::optional<float> capsuleLength;
		std::optional<float> lengthMult;
		std::optional<RE::NiTransform> transform;
		std::optional<RE::NiPoint3> groundShake;
		std::optional<TrailOverride> trailOverride;
	};
	
	enum class CollisionFilterComparisonResult : uint8_t
	{
		Continue,  // Do not affect whether the two objects should collide
		Collide,   // Force the two objects to collide
		Ignore,    // Force the two objects to not collide
	};

	enum class RequestedAttackCollisionType : uint8_t
	{
		Default,      // Return the largest currently active collision length, otherwise calculate right weapon's collision length
		Current,      // Return the largest currently active collision length, otherwise 0
		RightWeapon,  // Return either the length of the current right weapon collision if it exists, or calculate it
		LeftWeapon,   // Return either the length of the current left weapon collision if it exists, or calculate it
	};

	using PreHitCallback = std::function<PreHitCallbackReturn(const PrecisionHitData&)>;
	using PostHitCallback = std::function<void(const PrecisionHitData&, const RE::HitData&)>;
	using PrePhysicsStepCallback = std::function<void(RE::bhkWorld*)>;
	using CollisionFilterComparisonCallback = std::function<CollisionFilterComparisonResult(RE::bhkCollisionFilter*, uint32_t, uint32_t)>;
	using WeaponCollisionCallback = std::function<WeaponCollisionCallbackReturn(const PrecisionHitData&)>;
	using CollisionFilterSetupCallback = std::function<void(RE::bhkCollisionFilter*)>;
	using ContactListenerCallback = std::function<void(const RE::hkpContactPointEvent&)>;
	using PrecisionLayerSetupCallback = std::function<PrecisionLayerSetupCallbackReturn()>;

	// Precision's modder interface
	class IVPrecision1
	{
	public:
		/// <summary>
		/// Adds a callback that will run before Precision's hit function.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <param name="a_preHitCallback">The callback function</param>
		/// <returns>OK, AlreadyRegistered</returns>
		virtual APIResult AddPreHitCallback(SKSE::PluginHandle a_myPluginHandle, PreHitCallback&& a_preHitCallback) noexcept = 0;

		/// <summary>
		/// Adds a callback that will run after Precision's hit function.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <param name="a_postHitCallback">The callback function</param>
		/// <returns>OK, AlreadyRegistered</returns>
		virtual APIResult AddPostHitCallback(SKSE::PluginHandle a_myPluginHandle, PostHitCallback&& a_postHitCallback) noexcept = 0;

		/// <summary>
		/// Adds a callback that will run right before hkpWorld::stepDeltaTime is called.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <param name="a_prePhysicsStepCallback">The callback function</param>
		/// <returns>OK, AlreadyRegistered</returns>
		virtual APIResult AddPrePhysicsStepCallback(SKSE::PluginHandle a_myPluginHandle, PrePhysicsStepCallback&& a_prePhysicsStepCallback) noexcept = 0;

		/// <summary>
		/// Adds a callback that will run when havok compares collision filter info to determine if two objects should collide. This can be called hundreds of times per frame, so be brief.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <param name="a_collisionFilterComparisonCallback">The callback function</param>
		/// <returns>OK, AlreadyRegistered</returns>
		virtual APIResult AddCollisionFilterComparisonCallback(SKSE::PluginHandle a_myPluginHandle, CollisionFilterComparisonCallback&& a_collisionFilterComparisonCallback) noexcept = 0;

		/// <summary>
		/// Removes the callback that will run before Precision's hit function.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotRegistered</returns>
		virtual APIResult RemovePreHitCallback(SKSE::PluginHandle a_myPluginHandle) noexcept = 0;

		/// <summary>
		/// Removes the callback that will run after Precision's hit function.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotRegistered</returns>
		virtual APIResult RemovePostHitCallback(SKSE::PluginHandle a_myPluginHandle) noexcept = 0;

		/// <summary>
		/// Removes the callback that will run right before hkpWorld::stepDeltaTime is called.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotRegistered</returns>
		virtual APIResult RemovePrePhysicsStepCallback(SKSE::PluginHandle a_myPluginHandle) noexcept = 0;

		/// <summary>
		/// Removes the callback that will run when havok compares collision filter info to determine if two objects should collide.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotRegistered</returns>
		virtual APIResult RemoveCollisionFilterComparisonCallback(SKSE::PluginHandle a_myPluginHandle) noexcept = 0;

		/// <summary>
		/// Gets the current attack collision capsule length. In case of multiple active collisions, returns the largest. If there's no active collision, tries the best guess. Can be a bit complex so try not to call it every frame.
		/// </summary>
		/// <param name="a_actorHandle">Actor handle</param>
		/// <param name="a_collisionType">The type of collision to get</param>
		/// <returns>Capsule length</returns>
		virtual float GetAttackCollisionCapsuleLength(RE::ActorHandle a_actorHandle, RequestedAttackCollisionType a_collisionType = RequestedAttackCollisionType::Default) const noexcept = 0;
	};

	class IVPrecision2 : public IVPrecision1
	{
	public:
		/// <summary>
		/// Adds a callback that will run when two weapons collide.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <param name="a_callback">The callback function</param>
		/// <returns>OK, AlreadyRegistered</returns>
		virtual APIResult AddWeaponWeaponCollisionCallback(SKSE::PluginHandle a_myPluginHandle, WeaponCollisionCallback&& a_callback) noexcept = 0;

		/// <summary>
		/// Removes the callback that will run when two weapons collide.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotRegistered</returns>
		virtual APIResult RemoveWeaponWeaponCollisionCallback(SKSE::PluginHandle a_myPluginHandle) noexcept = 0;

		/// <summary>
		/// Adds a callback that will run when a weapon and a moving projectile collide.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <param name="a_callback">The callback function</param>
		/// <returns>OK, AlreadyRegistered</returns>
		virtual APIResult AddWeaponProjectileCollisionCallback(SKSE::PluginHandle a_myPluginHandle, WeaponCollisionCallback&& a_callback) noexcept = 0;

		/// <summary>
		/// Removes the callback that will run when a weapon and a moving projectile collide.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotRegistered</returns>
		virtual APIResult RemoveWeaponProjectileCollisionCallback(SKSE::PluginHandle a_myPluginHandle) noexcept = 0;

		[[deprecated("Use ApplyHitImpulse2 instead")]] virtual void ApplyHitImpulse(RE::ActorHandle a_actorHandle, RE::hkpRigidBody* a_rigidBody, const RE::NiPoint3& a_hitVelocity, const RE::hkVector4& a_hitPosition, float a_impulseMult) noexcept = 0;
	};

	class IVPrecision3 : public IVPrecision2
	{
	public:
		/// <summary>
		/// Adds a callback to where Precision alters the game's collision layers.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <param name="a_callback">The callback function</param>
		/// <returns>OK, AlreadyRegistered</returns>
		virtual APIResult AddCollisionFilterSetupCallback(SKSE::PluginHandle a_myPluginHandle, CollisionFilterSetupCallback&& a_callback) noexcept = 0;

		/// <summary>
		/// Removes the callback from where Precision alters the game's collision layers.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotRegistered</returns>
		virtual APIResult RemoveCollisionFilterSetupCallback(SKSE::PluginHandle a_myPluginHandle) noexcept = 0;

		/// <summary>
		/// Adds a callback to Precision's contact listener.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <param name="a_callback">The callback function</param>
		/// <returns>OK, AlreadyRegistered</returns>
		virtual APIResult AddContactListenerCallback(SKSE::PluginHandle a_myPluginHandle, ContactListenerCallback&& a_callback) noexcept = 0;

		/// <summary>
		/// Removes the callback from Precision's contact listener.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotRegistered</returns>
		virtual APIResult RemoveContactListenerCallback(SKSE::PluginHandle a_myPluginHandle) noexcept = 0;

		/// <summary>
		/// Checks if the actor is "active" in Precision's system (has ragdoll added, etc.)
		/// </summary>
		/// <param name="a_actorHandle">Actor handle</param>
		/// <returns>Whether the actor is active</returns>
		[[nodiscard]] virtual bool IsActorActive(RE::ActorHandle a_actorHandle) const noexcept = 0;

		/// <summary>
		/// Checks if the actor is "active" in Precision's system (has ragdoll added, etc.)
		/// </summary>
		/// <param name="a_collisionGroup">Collision group to check</param>
		/// <returns>Whether the actor is active</returns>
		[[nodiscard]] virtual bool IsActorActiveCollisionGroup(uint16_t a_collisionGroup) const noexcept = 0;

		/// <summary>
		/// Checks if the actor's character controller is considered hittable by Precision (the actor is either not "active", or their ragdoll has no collision - e.g. wisps)
		/// </summary>
		/// <param name="a_actorHandle">Actor handle</param>
		/// <returns>Whether the actor's character controller is hittable</returns>
		[[nodiscard]] virtual bool IsActorCharacterControllerHittable(RE::ActorHandle a_actorHandle) const noexcept = 0;

		/// <summary>
		/// Checks if the character controller is considered hittable by Precision (the actor is either not "active", or their ragdoll has no collision - e.g. wisps)
		/// </summary>
		/// <param name="a_characterController">Character controller</param>
		/// <returns>Whether the character controller is hittable</returns>
		[[nodiscard]] virtual bool IsCharacterControllerHittable(RE::bhkCharacterController* a_characterController) const noexcept = 0;

		/// <summary>
		/// Checks if the character controller is considered hittable by Precision (the actor is either not "active", or their ragdoll has no collision - e.g. wisps)
		/// </summary>
		/// <param name="a_collisionGroup">Collision group to check</param>
		/// <returns>Whether the character controller is hittable</returns>
		[[nodiscard]] virtual bool IsCharacterControllerHittableCollisionGroup(uint16_t a_collisionGroup) const noexcept = 0;

		/// <summary>
		/// Disables Precision for an actor (removes the ragdoll etc.)
		/// </summary>
		/// <param name="a_actorHandle">Actor handle</param>
		/// <param name="a_bDisable">Whether to disable or remove from the disable list</param>
		/// <returns>Whether the actor was successfully added or removed from the disable list</returns>
		virtual bool ToggleDisableActor(RE::ActorHandle a_actorHandle, bool a_bDisable) noexcept = 0;
	};

	class IVPrecision4 : public IVPrecision3
	{
	public:
		/// <summary>
		/// Adds a callback that can add/remove layers that should collide with Precision Layers.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <param name="a_callback">The callback function</param>
		/// <returns>OK, AlreadyRegistered</returns>
		virtual APIResult AddPrecisionLayerSetupCallback(SKSE::PluginHandle a_myPluginHandle, PrecisionLayerSetupCallback&& a_callback) noexcept = 0;

		/// <summary>
		/// Removes the callback that can add/remove layers that should collide with Precision Layers.
		/// </summary>
		/// <param name="a_myPluginHandle">Your assigned plugin handle</param>
		/// <returns>OK, NotRegistered</returns>
		virtual APIResult RemovePrecisionLayerSetupCallback(SKSE::PluginHandle a_myPluginHandle) noexcept = 0;

		/// <summary>
		/// Returns the original node in case the given node is a clone.
		/// </summary>
		/// <param name="a_actorHandle">Actor handle</param>
		/// <param name="a_node">Node that's potentially a clone</param>
		/// <returns>The original node, or nullptr if the given one wasn't a clone</returns>
		virtual RE::NiAVObject* GetOriginalFromClone(RE::ActorHandle a_actorHandle, RE::NiAVObject* a_node) noexcept = 0;

		/// <summary>
		/// Returns the original rigid body in case the given rigid body is a clone.
		/// </summary>
		/// <param name="a_actorHandle">Actor handle</param>
		/// <param name="a_hkpRigidBody">Rigid body that's potentially a clone</param>
		/// <returns>The original rigid body, or nullptr if the given one wasn't a clone</returns>
		virtual RE::hkpRigidBody* GetOriginalFromClone(RE::ActorHandle a_actorHandle, RE::hkpRigidBody* a_hkpRigidBody) noexcept = 0;

		/// <summary>
		/// Applies impulse.
		/// </summary>
		/// <param name="a_targetActorHandle">Target actor handle</param>
		/// <param name="a_sourceActorHandle">Source actor handle</param>
		/// <param name="a_rigidBody">Hit rigid body</param>
		/// <param name="a_hitVelocity">Hit velocity vector</param>
		/// <param name="a_hitPosition">Hit position</param>
		/// <param name="a_impulseMult">Impulse strength multiplier</param>
		virtual void ApplyHitImpulse2(RE::ActorHandle a_targetActorHandle, RE::ActorHandle a_sourceActorHandle, RE::hkpRigidBody* a_rigidBody, const RE::NiPoint3& a_hitVelocity, const RE::hkVector4& a_hitPosition, float a_impulseMult) noexcept = 0;

		/// <summary>
		/// Adds a trail to the node.
		/// </summary>
		/// <param name="a_trailParentNode">Node that trail will follow</param>
		/// <param name="a_sourceActorHandle">Source actor</param>
		/// <param name="a_sourceActorParentCell">Source actor cell</param>
		/// <param name="a_weaponItem">Weapon</param>
		/// <param name="a_bIsLeftHand">Is left hand object?</param>
		/// <param name="a_bTrailUseTrueLength">Is trail use true lenght?</param>
		/// <param name="a_trailOverride">Trail override</param>
		virtual void AddAttackTrail(RE::NiNode* a_trailParentNode, RE::ActorHandle a_sourceActorHandle, RE::TESObjectCELL* a_sourceActorParentCell, RE::InventoryEntryData* a_weaponItem, bool a_bIsLeftHand, bool a_bTrailUseTrueLength, std::optional<TrailOverride> a_trailOverride) noexcept = 0;

		/// <summary>
		/// Adds a trail to the node.
		/// </summary>
		/// <param name="a_trailParentNode">Node that trail will follow</param>
		/// <param name="a_sourceActorHandle">Source actor</param>
		/// <param name="a_sourceActorParentCell">Source actor cell</param>
		/// <param name="a_projectile">Projectile</param>
		/// <param name="a_trailOverride">Trail override</param>
		virtual void AddAttackTrail(RE::NiNode* a_trailParentNode, RE::ActorHandle a_sourceActorHandle, RE::TESObjectCELL* a_sourceActorParentCell, RE::Projectile* a_projectile, std::optional<TrailOverride> a_trailOverride) noexcept = 0;

		virtual void AddAttackCollision(RE::ActorHandle a_actorHandle, const RE::BSAnimationGraphEvent a_event) noexcept = 0;
		virtual void AddAttackCollision(RE::ActorHandle a_actorHandle, CollisionDefinition& a_collisionDefinition, RE::Projectile* a_projectile) noexcept = 0;
		virtual bool RemoveAttackCollision(RE::ActorHandle a_actorHandle, const CollisionDefinition& a_collisionDefinition) noexcept = 0;
		virtual bool RemoveProjectileCollision(RE::ActorHandle a_actorHandle, const CollisionDefinition& a_collisionDefinition) noexcept = 0;
	};

	typedef void* (*_RequestPluginAPI)(const InterfaceVersion interfaceVersion);

	/// <summary>
	/// Request the Precision API interface.
	/// Recommended: Send your request when you need to use the API and cache the pointer. SKSEMessagingInterface::kMessage_PostLoad seems to be unreliable for some users for unknown reasons.
	/// </summary>
	/// <param name="a_interfaceVersion">The interface version to request</param>
	/// <returns>The pointer to the API singleton, or nullptr if request failed</returns>
	[[nodiscard]] inline void* RequestPluginAPI(const InterfaceVersion a_interfaceVersion = InterfaceVersion::V4)
	{
		auto pluginHandle = GetModuleHandleA("Precision.dll");
		_RequestPluginAPI requestAPIFunction = (_RequestPluginAPI)GetProcAddress(pluginHandle, "RequestPluginAPI");
		if (requestAPIFunction) {
			return requestAPIFunction(a_interfaceVersion);
		}
		return nullptr;
	}
}
