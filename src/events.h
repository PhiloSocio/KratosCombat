class AnimationEventTracker : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
public:
    static AnimationEventTracker* GetSingleton() {static AnimationEventTracker singleton; return &singleton;}

    static bool Register();

    virtual EventChecker ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) override;
};
class AnimObjectAnimationEventTracker : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
public:
    static AnimObjectAnimationEventTracker* GetSingleton() {static AnimObjectAnimationEventTracker singleton; return &singleton;}

    static bool Register();

    virtual EventChecker ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) override;
};
class MagicEffectApplyTracker : public RE::BSTEventSink<RE::TESMagicEffectApplyEvent>
{
public:
    static MagicEffectApplyTracker* GetSingleton() {static MagicEffectApplyTracker singleton; return &singleton;}

    static bool Register();

    virtual EventChecker ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>* a_eventSource) override;
};
class InputEventTracker : public RE::BSTEventSink<RE::InputEvent*>
{
public:
    static InputEventTracker* GetSingleton() {static InputEventTracker singleton; return &singleton;}

    static bool Register();

    virtual EventChecker ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override;
private:
    enum : std::uint32_t
    {
        kInvalid = static_cast<std::uint32_t>(-1),
        kKeyboardOffset = 0,
        kMouseOffset = 256,
        kGamepadOffset = 266,
    };
    static std::uint32_t GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key);
    std::uint32_t GetOffsettedKeyCode(std::uint32_t a_keyCode, RE::INPUT_DEVICE a_inputDevice) const;

    InputEventTracker() = default;
    InputEventTracker(const InputEventTracker&) = delete;
    InputEventTracker(InputEventTracker&&) = delete;
    virtual ~InputEventTracker() = default;
    InputEventTracker& operator=(const InputEventTracker&) = delete;
    InputEventTracker& operator=(InputEventTracker&&) = delete;
};
class MenuOpenCloseTracker : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    static MenuOpenCloseTracker* GetSingleton() {static MenuOpenCloseTracker singleton; return &singleton;}

    static bool Register();

    virtual EventChecker ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) override;
};
inline bool RegisterEvents() 
{
    return !(
        !AnimationEventTracker::Register() ||
    //    !AnimObjectAnimationEventTracker::Register() ||
        !MagicEffectApplyTracker::Register() ||
        !MenuOpenCloseTracker::Register() ||
        !InputEventTracker::Register()
    );
}