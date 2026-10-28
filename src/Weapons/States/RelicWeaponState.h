#pragma once

class RelicWeaponState
{
public:
    enum class Type : std::uint8_t {
        kNone,
        kArriving,
        kHoming
    };

    enum class Status : std::uint8_t {
        kRunning,
        kCompleted,
        kCancelled
    };

    enum class Process : std::uint8_t {
        kMain,
        kAttack,
        kTransition
    };

    virtual ~RelicWeaponState() = default;

    virtual void Enter() = 0;
    virtual Status Update(float a_delta) = 0;
    virtual void Exit() = 0;
    
    virtual void OnHit(RE::Actor* a_victim) = 0;

    [[nodiscard]] virtual Type GetType() const = 0;

    template <typename T>
    [[nodiscard]] T* As() noexcept
    {
        return const_cast<T*>(static_cast<const RelicWeaponState*>(this)->As<T>());
    }
    template <typename T>
    [[nodiscard]] const T* As() const noexcept
    {
        static_assert(std::is_base_of_v<RelicWeaponState, T>);
        static_assert(requires { T::kType; });
        return GetType() == T::kType ? static_cast<const T*>(this) : nullptr;
    }
};

using Status = RelicWeaponState::Status;
using Type = RelicWeaponState::Type;
