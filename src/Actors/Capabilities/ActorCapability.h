#pragma once

#include "Types.h"

class BaseActor;

class ActorCapability
{
public:
    virtual ~ActorCapability() = default;

    virtual void OnAttach(BaseActor* a_actor) { parent = a_actor; }
    virtual void OnDetach(BaseActor* a_actor) { parent = nullptr; }

    virtual void Update(float a_delta) {}
    virtual void HandleAction(const ActionType a_action) {}

    [[nodiscard]] virtual ActorType GetType() const = 0;
    [[nodiscard]] virtual bool IsActive() const { return true; }

    [[nodiscard]] BaseActor* GetParent() const { return parent; }

    template <typename T>
    [[nodiscard]] T* As() noexcept
    {
        return const_cast<T*>(static_cast<const ActorCapability*>(this)->As<T>());
    }
    template <typename T>
    [[nodiscard]] const T* As() const noexcept
    {
        static_assert(std::is_base_of_v<ActorCapability, T>);
        static_assert(requires { T::kType; });
        return GetType() == T::kType ? static_cast<const T*>(this) : nullptr;
    }

protected:
    BaseActor* parent = nullptr;
};

using ActorCapabilityPtr = std::unique_ptr<ActorCapability>;