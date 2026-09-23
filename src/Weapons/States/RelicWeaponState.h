#pragma once

class RelicWeaponState
{
public:
    enum class Type
    {
        kNone,
        kArriving,
        kHoming
    };

    enum class Status
    {
        kRunning,
        kCompleted,
        kCancelled
    };

    virtual ~RelicWeaponState() = default;

    virtual void Enter() = 0;
    virtual Status Update(float a_delta) = 0;
    virtual void Exit() = 0;

    [[nodiscard]] virtual Type GetType() const = 0;
};
using Status = RelicWeaponState::Status;
using Type = RelicWeaponState::Type;
