#pragma once


#define PI 3.1415926535897932f
#define TWOTHIRDS_PI 2.0943951023931955f
#define TWO_PI 6.2831853071795865f
#define PI2 1.5707963267948966f
#define PI3 1.0471975511965977f
#define PI4 0.7853981633974483f
#define PI8 0.3926990816987242f

#define downVec {0.f, 0.f, -1.f}
#define upVec   {0.f, 0.f, 1.f}
#define frontVec    {0.f, 1.f, 0.f}
#define backVec     {0.f, -1.f, 0.f}
#define rightVec    {1.f, 0.f, 0.f}
#define leftVec     {-1.f, 0.f, 0.f}

static float* g_deltaTime = (float*)RELOCATION_ID(523660, 410199).address();            //  sensitive to slow time spell
static float* g_deltaTimeRealTime = (float*)RELOCATION_ID(523661, 410200).address();    //  const
static float* g_engineTime = (float*)RELOCATION_ID(517597, 404125).address();           //  credits to https://github.com/jarari

using namespace RE;

namespace FenixUtils {  //credits to master fenix https://github.com/fenix31415/UselessFenixUtils
    static void stagger(float val, RE::Actor* victim, RE::Actor* attacker = nullptr)
    {
        if (victim) {
            float stagDir = 0.0f;
            if (attacker && victim->GetHandle() != attacker->GetHandle()) {
                auto heading = victim->GetHeadingAngle(attacker->GetPosition(), false);
                stagDir = (heading >= 0.0f) ? heading / 360.0f : (360.0f + heading) / 360.0f;
            }

            victim->SetGraphVariableFloat("staggerDirection", stagDir);
            victim->SetGraphVariableFloat("staggerMagnitude", val);
            victim->NotifyAnimationGraph("staggerStart");
        }
    }
    static void stagger(float val, RE::Actor* victim, RE::Projectile* hitProjectile = nullptr)
    {
        if (victim) {
            float stagDir = 0.0f;
            if (hitProjectile) {
                stagDir = hitProjectile->GetHeadingAngle(victim->GetPosition(), false);
            }
            stagDir = (stagDir >= 0.0f) ? stagDir / 360.0f : (360.0f + stagDir) / 360.0f;

            victim->SetGraphVariableFloat("staggerDirection", stagDir);
            victim->SetGraphVariableFloat("staggerMagnitude", val);
            victim->NotifyAnimationGraph("staggerStart");
        }
    }
}
namespace PointerUtil //yoinked po3's code
{
template <class T, class U>
inline auto adjust_pointer(U* a_ptr, std::ptrdiff_t a_adjust) noexcept
{
    auto addr = a_ptr ? reinterpret_cast<std::uintptr_t>(a_ptr) + a_adjust : 0;
    if constexpr (std::is_const_v<U> && std::is_volatile_v<U>) {
        return reinterpret_cast<std::add_cv_t<T>*>(addr);
    } else if constexpr (std::is_const_v<U>) {
        return reinterpret_cast<std::add_const_t<T>*>(addr);
    } else if constexpr (std::is_volatile_v<U>) {
        return reinterpret_cast<std::add_volatile_t<T>*>(addr);
    } else {
        return reinterpret_cast<T*>(addr);
    }
}
}
namespace AsyncUtil
{
    class GameTime
    {
    public:
        GameTime() = default;
        static float GetEngineTime() {return *g_engineTime;}
        void RegisterForUpdate(const float a_delaySeconds, const bool a_single = true) {_done = false; _single = a_single; _registerTime = GetEngineTime(); _updateTime = _registerTime + a_delaySeconds;}
        float GetUpdateTime() {return _updateTime;}
        bool IsTimeToUpdate() {if (!_done && GetEngineTime() >= _updateTime) {if (_single) _done = true; return true;} else return false;}
        void Done() {_done = true;}
    private:
        float _registerTime;
        float _updateTime;
        bool _single;
        bool _done = true;
    };
}
namespace SystemUtil
{
    struct File 
    {
        static std::vector<std::string> GetConfigs(std::string_view a_folder, std::string_view a_suffix, std::string_view a_extension = ".ini"sv)
        {
            std::vector<std::string> configs{};

            for (const auto iterator = std::filesystem::directory_iterator(a_folder); const auto& entry : iterator) {
                if (entry.exists()) {
                    if (const auto& path = entry.path(); !path.empty() && path.extension() == a_extension) {
                        if (const auto& fileName = entry.path().string(); fileName.rfind(a_suffix) != std::string::npos) {
                            configs.push_back(fileName);
                        }
                    }
                }
            }

            std::ranges::sort(configs);

            return configs;
        }
    };
}

namespace KeyUtil 
{

    enum class MACRO_LIMITS {
        kMaxMacros = 282
    };

    enum class KBM_OFFSETS {
        // first 256 for keyboard, then 8 mouse buttons, then mouse wheel up, wheel down, then 16 gamepad buttons
        kMacro_KeyboardOffset = 0,      // not actually used, just for self-documentation
        kMacro_NumKeyboardKeys = 256,

        kMacro_MouseButtonOffset = kMacro_NumKeyboardKeys,  // 256
        kMacro_NumMouseButtons = 8,

        kMacro_MouseWheelOffset = kMacro_MouseButtonOffset + kMacro_NumMouseButtons,    // 264
        kMacro_MouseWheelDirections = 2,

        kMacro_GamepadOffset = kMacro_MouseWheelOffset + kMacro_MouseWheelDirections,   // 266
        kMacro_NumGamepadButtons = 16,

            // 282
    };

    enum class GAMEPAD_OFFSETS {
        kGamepadButtonOffset_DPAD_UP = static_cast<int>(KBM_OFFSETS::kMacro_GamepadOffset), // 266
        kGamepadButtonOffset_DPAD_DOWN,
        kGamepadButtonOffset_DPAD_LEFT,
        kGamepadButtonOffset_DPAD_RIGHT,
        kGamepadButtonOffset_START,
        kGamepadButtonOffset_BACK,
        kGamepadButtonOffset_LEFT_THUMB,
        kGamepadButtonOffset_RIGHT_THUMB,
        kGamepadButtonOffset_LEFT_SHOULDER,
        kGamepadButtonOffset_RIGHT_SHOULDER,
        kGamepadButtonOffset_A,
        kGamepadButtonOffset_B,
        kGamepadButtonOffset_X,
        kGamepadButtonOffset_Y,
        kGamepadButtonOffset_LT,
        kGamepadButtonOffset_RT // 281
    };


    struct Interpreter
    {
        public: 
        static uint32_t GamepadMaskToKeycode(uint32_t keyMask) {
    switch (keyMask) {
        case 0x001:     return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_DPAD_UP);
        case 0x002:     return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_DPAD_DOWN);
        case 0x004:     return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_DPAD_LEFT);
        case 0x008:     return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_DPAD_RIGHT);
        case 0x0010:    return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_START);
        case 0x0020:    return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_BACK);
        case 0x0040:    return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_LEFT_THUMB);
        case 0x0080:    return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_RIGHT_THUMB);
        case 0x0100:    return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_LEFT_SHOULDER);
        case 0x0200:    return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_RIGHT_SHOULDER);
        case 0x1000:    return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_A);
        case 0x2000:    return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_B);
        case 0x4000:    return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_X);
        case 0x8000:    return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_Y);
        case 0x9:       return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_LT);
        case 0xA:       return static_cast<int>(GAMEPAD_OFFSETS::kGamepadButtonOffset_RT);
        default:        return 282; // Invalid
    }
}

    };

}

namespace Util
{
    struct String
    {
    //    static std::vector<std::string> Split(const std::string& a_str, std::string_view a_delimiter)
    //    {
    //        auto range = a_str | std::ranges::views::split(a_delimiter) | std::ranges::views::transform([](auto&& r) { return std::string_view(r); });
    //        return { range.begin(), range.end() };
    //    }



        static bool iContains(std::string_view a_str1, std::string_view a_str2)
        {
            if (a_str2.length() > a_str1.length()) {
                return false;
            }

            const auto subrange = std::ranges::search(a_str1, a_str2, [](unsigned char ch1, unsigned char ch2) {
                return std::toupper(ch1) == std::toupper(ch2);
            });

            return !subrange.empty();
        }

        static bool iEquals(std::string_view a_str1, std::string_view a_str2)
        {
            return std::ranges::equal(a_str1, a_str2, [](unsigned char ch1, unsigned char ch2) {
                return std::toupper(ch1) == std::toupper(ch2);
            });
        }

        // https://stackoverflow.com/a/35452044
        static std::string Join(const std::vector<std::string>& a_vec, std::string_view a_delimiter)
        {
            return std::accumulate(a_vec.begin(), a_vec.end(), std::string{},
                [a_delimiter](const auto& str1, const auto& str2) {
                    return str1.empty() ? str2 : str1 + a_delimiter.data() + str2;
                });
        }

        static std::vector<float> ToFloatVector(const std::vector<std::string> stringVector)
        {
            std::vector<float> floatNumbers; 
            for(auto str : stringVector)
            {
                float num = atof(str.c_str());
                floatNumbers.push_back(num);
            }
            return floatNumbers;
        }
        static std::string ToLower(std::string_view a_str)
        {
            std::string result(a_str);
            std::ranges::transform(result, result.begin(), [](unsigned char ch) { return static_cast<unsigned char>(std::tolower(ch)); });
            return result;
        }

        static std::string ToUpper(std::string_view a_str)
        {
            std::string result(a_str);
            std::ranges::transform(result, result.begin(), [](unsigned char ch) { return static_cast<unsigned char>(std::toupper(ch)); });
            return result;
        }
    };
    
    constexpr uint32_t hash(const char* data, size_t const size) noexcept
    {
        uint32_t hash = 5381;

        for (const char* c = data; c < data + size; ++c) {
            hash = ((hash << 5) + hash) + (unsigned char)*c;
        }

        return hash;
    }

    constexpr uint32_t operator""_h(const char* str, size_t size) noexcept
    {
        return hash(str, size);
    }

}

namespace MathUtil
{
    struct Angle 
    {
        [[nodiscard]] constexpr static float DegreeToRadian(float a_angle)
        {
            return a_angle * 0.017453292f;
        }

        [[nodiscard]] constexpr static float RadianToDegree(float a_radian)
        {
            return a_radian * 57.295779513f;
        }

        static NiPoint3 ToRadianVector(float x, float y, float z)
        {
            RE::NiPoint3 rotationVector{ 0.f, 0.f, 0.f };

            rotationVector.x = DegreeToRadian(x); 
            rotationVector.y = DegreeToRadian(y); 
            rotationVector.z = DegreeToRadian(z); 
            return rotationVector; 
        }

        static float NormalAbsoluteAngle(float a_angle)
        {
            while (a_angle < 0)
                a_angle += TWO_PI;
            while (a_angle > TWO_PI)
                a_angle -= TWO_PI;
            return a_angle;

            // return fmod(a_angle, TWO_PI) >= 0 ? a_angle : (a_angle + TWO_PI);
        }

        static float NormalRelativeAngle(float a_angle)
        {
            while (a_angle > PI)
                a_angle -= TWO_PI;
            while (a_angle < -PI)
                a_angle += TWO_PI;
            return a_angle;

            // return fmod(a_angle, TWO_PI) >= 0 ? (a_angle < PI) ? a_angle : a_angle - TWO_PI : (a_angle >= -PI) ? a_angle : a_angle + TWO_PI;
        }

        static NiPoint3 BlendVectors(const NiPoint3 a, const NiPoint3 b, float a_to_b_ratio, const bool a_normalize = false)
        {
            a_to_b_ratio = std::clamp(a_to_b_ratio, 0.0f, 1.0f);

            RE::NiPoint3 v = (a * (1.0f - a_to_b_ratio) + b * a_to_b_ratio);
            if (a_normalize) v.Unitize();

            return v;
        }
    }; 

    struct Algebra
    {
        [[nodiscard]] inline static RE::NiPoint3 RotateVectorRodrigues(const RE::NiPoint3& v, const RE::NiPoint3& rotatingAxis, float theta)
        {
            float cosTheta = std::cos(theta);
            float sinTheta = std::sin(theta);

            return v * cosTheta +
                rotatingAxis.Cross(v) * sinTheta +
                rotatingAxis * (rotatingAxis.Dot(v)) * (1.f - cosTheta);
        }
        [[nodiscard]] inline static RE::NiQuaternion QuaternionWithAngleAxis(float angleRadians, const RE::NiPoint3& axis)
        {
            RE::NiPoint3 normalizedAxis = axis;
            normalizedAxis.Unitize();

            float halfAngle = angleRadians * 0.5f;
            float sinHalf = std::sin(halfAngle);
            float cosHalf = std::cos(halfAngle);

            return RE::NiQuaternion{
                cosHalf,                    // w
                normalizedAxis.x * sinHalf, // x
                normalizedAxis.y * sinHalf, // y
                normalizedAxis.z * sinHalf  // z
            };
        }
        [[nodiscard]] inline static RE::NiQuaternion QuaternionWithAngleAxis(float angleRadians, float x, float y, float z)
        {
            return QuaternionWithAngleAxis(angleRadians, RE::NiPoint3{ x, y, z });
        }
        [[nodiscard]] inline static RE::NiMatrix3 QuaternionToMatrix(const RE::NiQuaternion& q)
        {
            RE::NiQuaternion normQ = q;
            Unitize(normQ); // normalize etmek şart — dönüş matrisinin düzgün olması için

            float x = normQ.x, y = normQ.y, z = normQ.z, w = normQ.w;
            
            float xx = x * x, yy = y * y, zz = z * z;
            float xy = x * y, xz = x * z, yz = y * z;
            float wx = w * x, wy = w * y, wz = w * z;

            RE::NiMatrix3 m;
            m.entry[0][0] = 1.f - 2.f * (yy + zz);
            m.entry[0][1] = 2.f * (xy - wz);
            m.entry[0][2] = 2.f * (xz + wy);

            m.entry[1][0] = 2.f * (xy + wz);
            m.entry[1][1] = 1.f - 2.f * (xx + zz);
            m.entry[1][2] = 2.f * (yz - wx);

            m.entry[2][0] = 2.f * (xz - wy);
            m.entry[2][1] = 2.f * (yz + wx);
            m.entry[2][2] = 1.f - 2.f * (xx + yy);

            return m;
        }
        [[nodiscard]] inline static void QuaternionToAngleAxis(const RE::NiQuaternion& q, float& outAngle, RE::NiPoint3& outAxis)
        {
            RE::NiQuaternion normQ = q;
            Unitize(normQ);

            outAngle = 2.0f * std::acos(normQ.w);

            float s = std::sqrt(1.0f - normQ.w * normQ.w);
            if (s < 1e-6f) {
                // Sıfıra çok yakınsa yön rastgele olabilir
                outAxis = RE::NiPoint3{1.f, 0.f, 0.f}; // default
            } else {
                outAxis = RE::NiPoint3{
                    normQ.x / s,
                    normQ.y / s,
                    normQ.z / s
                };
            }
        }
        inline static void Unitize(RE::NiQuaternion& q)
        {
            float lenSqr = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;

            if (lenSqr > 1e-6f) {
                float invLen = 1.0f / std::sqrt(lenSqr);
                q.w *= invLen;
                q.x *= invLen;
                q.y *= invLen;
                q.z *= invLen;
            } else {
                // Geçersiz dönüş, identity quaternion’a sıfırla
                q.w = 1.f;
                q.x = q.y = q.z = 0.f;
            }
        }
        [[nodiscard]] inline static RE::NiQuaternion MultiplyQuaternions(const RE::NiQuaternion& a, const RE::NiQuaternion& b)
        {
            return {
                a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,                 // w
                a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,                 // x
                a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,                 // y
                a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w                  // z
            };
        }

        [[nodiscard]] inline static float ParabolicClamp(float t, float minVal, float maxVal) {
            float factor = 4.0f * t * (1.0f - t); // 0 ve 1'de min, 0.5'te max
            return minVal + (maxVal - minVal) * factor;
        }

        [[nodiscard]] inline static RE::NiPoint3 BezierPoint(float t, const RE::NiPoint3& P0, const RE::NiPoint3& P1, const RE::NiPoint3& P2, const RE::NiPoint3& P3) {
            // t is expected to be in [0, 1]
            t = std::clamp(t, 0.0f, 1.0f);
            const float u = 1.0f - t;
            const float uu = u * u;
            const float uuu = uu * u;
            const float tt = t * t;
            const float ttt = tt * t;

            return  P0 * uuu + 
                    P1 * (3.f * uu * t) + 
                    P2 * (3.f * u * tt) + 
                    P3 * ttt;
        }

        [[nodiscard]] static float BezierCurveLength(const RE::NiPoint3& P0, const RE::NiPoint3& P1, const RE::NiPoint3& P2, const RE::NiPoint3& P3, int subdivisions) {
            if (subdivisions <= 0)
                return 0.0f;

            float length = 0.0f;
            RE::NiPoint3 prevPoint = P0;
            const float invSubdiv = 1.0f / static_cast<float>(subdivisions);

            for (int i = 1; i <= subdivisions; ++i) {
                const float t = static_cast<float>(i) * invSubdiv;
                const RE::NiPoint3 currentPoint = BezierPoint(t, P0, P1, P2, P3);
                length += (currentPoint - prevPoint).Length();
                prevPoint = currentPoint;
            }

            return length;
        }

        [[nodiscard]] static std::pair<std::vector<RE::NiPoint3>, float>
        DrawAndMeasureBezier(const RE::NiPoint3& P0, const RE::NiPoint3& P1, const RE::NiPoint3& P2, const RE::NiPoint3& P3, int subdivisions)
        {
            std::vector<RE::NiPoint3> points;
            points.reserve(subdivisions + 1);

            float length = 0.0f;
            RE::NiPoint3 prevPoint = P0;
            points.push_back(P0);

            const float invSubdiv = 1.0f / static_cast<float>(subdivisions);

            for (int i = 1; i <= subdivisions; ++i) {
                float t = static_cast<float>(i) * invSubdiv;
                RE::NiPoint3 currentPoint = MathUtil::Algebra::BezierPoint(t, P0, P1, P2, P3);
                length += (currentPoint - prevPoint).Length();
                points.push_back(currentPoint);
                prevPoint = currentPoint;
            }

            return {points, length};
        }

        [[nodiscard]] static float AttractToNearest(const float value, const std::vector<float>& targets, const float strength)
        {
            if (targets.empty()) return value;

            float closest = targets[0];
            float minDist = std::abs(value - closest);

            for (float target : targets) {
                float dist = std::abs(value - target);
                if (dist < minDist) {
                    closest = target;
                    minDist = dist;
                }
            }

            const float t = std::clamp(strength, 0.f, 1.f);
            return value * (1.f - t) + closest * t;
        }

        [[nodiscard]] static RE::Projectile::ProjectileRot VectorToPitchYaw(const float x, const float y, const float z) noexcept
        {
            RE::Projectile::ProjectileRot angles;

            // Calculate yaw (rotation around Z-axis in the XY plane)
            angles.z = std::atan2(x, y);

            // Calculate pitch (rotation up/down along the YZ plane)
            angles.x = std::atan2(-z, std::sqrt(x * x + y * y));
        //    angles.x = std::asin(-y);

            return angles;
        }

        [[nodiscard]] static RE::Projectile::ProjectileRot VectorToPitchYaw(const RE::NiPoint3 a_oriention) noexcept
        {
            return VectorToPitchYaw(a_oriention.x, a_oriention.y, a_oriention.z);
        }

        [[nodiscard]] static RE::NiPoint3 PitchYawToVector(const float a_pitch, const float a_yaw) {
            float cp = std::cos(a_pitch);
            float sp = std::sin(a_pitch);
            float cy = std::cos(a_yaw);
            float sy = std::sin(a_yaw);

            return RE::NiPoint3(cp * sy, cp * cy, -sp);
        }

        [[nodiscard]] static RE::NiPoint3 PitchYawToVector(const RE::Projectile::ProjectileRot a_rotation) noexcept
        {
            return PitchYawToVector(a_rotation.x, a_rotation.z);
        }

        static inline RE::NiPoint3 GetForwardVector(RE::Actor* a_actor)
        {
            RE::NiPoint3 forwardVector;
            if (a_actor) {
                const float yaw = a_actor->data.angle.z;
                forwardVector.x = std::sin(yaw);
                forwardVector.y = std::cos(yaw);
                forwardVector.z = 0.0f;
            }   return forwardVector;
        }

        [[nodiscard]] static inline float GenerateRandomFloat(const float lower, const float upper) {
            static std::default_random_engine generator;
            std::uniform_real_distribution<float> distribution(lower, upper);
                return distribution(generator);
        }
        [[nodiscard]] static inline void SetRotationMatrix(RE::NiMatrix3& a_matrix, const float sacb, const float cacb, const float sb) {
            const float cb = std::sqrtf(1 - sb * sb);
            const float ca = cacb / cb;
            const float sa = sacb / cb;
            a_matrix.entry[0][0] = ca;
            a_matrix.entry[1][0] = sa;
            a_matrix.entry[2][0] = 0.0;
            a_matrix.entry[0][1] = -sacb;
            a_matrix.entry[1][1] = cacb;
            a_matrix.entry[2][1] = sb;
            a_matrix.entry[0][2] = sa * sb;
            a_matrix.entry[1][2] = -ca * sb;
            a_matrix.entry[2][2] = cb;
        }
        [[nodiscard]] static void RotateMatrixAroundAxis(NiMatrix3& a_matrix, const float angleRad, const char* axis) {
            const float cosA = std::cos(angleRad);
            const float sinA = std::sin(angleRad);
            if (std::strcmp(axis, "x") == 0) {
                a_matrix.entry[0][0] = 1.0;
                a_matrix.entry[1][1] = cosA;
                a_matrix.entry[1][2] = -sinA;
                a_matrix.entry[2][1] = sinA;
                a_matrix.entry[2][2] = cosA;
            } else if (std::strcmp(axis, "y") == 0) {
                a_matrix.entry[0][0] = cosA;
                a_matrix.entry[0][2] = sinA;
                a_matrix.entry[1][1] = 1.0;
                a_matrix.entry[2][0] = -sinA;
                a_matrix.entry[2][2] = cosA;
            } else if (std::strcmp(axis, "z") == 0) {
                a_matrix.entry[0][0] = cosA;
                a_matrix.entry[0][1] = -sinA;
                a_matrix.entry[1][0] = sinA;
                a_matrix.entry[1][1] = cosA;
                a_matrix.entry[2][2] = 1.0;
            } else {
                return;
            }
        }
        [[nodiscard]] static void RotateMatrixAroundAxisses(NiMatrix3& a_matrix, float angleRadX, float angleRadY, float angleRadZ) {
            float cosX = std::cos(angleRadX);
            float sinX = std::sin(angleRadX);
            float cosY = std::cos(angleRadY);
            float sinY = std::sin(angleRadY);
            float cosZ = std::cos(angleRadZ);
            float sinZ = std::sin(angleRadZ);
                // X ekseninde döndürme matrisi
            NiMatrix3 rotationMatrixX;
            rotationMatrixX.entry[0][0] = 1.0;
            rotationMatrixX.entry[1][1] = cosX;
            rotationMatrixX.entry[1][2] = -sinX;
            rotationMatrixX.entry[2][1] = sinX;
            rotationMatrixX.entry[2][2] = cosX;

            // Y ekseninde döndürme matrisi
            NiMatrix3 rotationMatrixY;
            rotationMatrixY.entry[0][0] = cosY;
            rotationMatrixY.entry[0][2] = sinY;
            rotationMatrixY.entry[1][1] = 1.0;
            rotationMatrixY.entry[2][0] = -sinY;
            rotationMatrixY.entry[2][2] = cosY;

            // Z ekseninde döndürme matrisi
            NiMatrix3 rotationMatrixZ;
            rotationMatrixZ.entry[0][0] = cosZ;
            rotationMatrixZ.entry[0][1] = -sinZ;
            rotationMatrixZ.entry[1][0] = sinZ;
            rotationMatrixZ.entry[1][1] = cosZ;
            rotationMatrixZ.entry[2][2] = 1.0;

            // Orijinal matris ile çarpma
            a_matrix = a_matrix * rotationMatrixX * rotationMatrixY * rotationMatrixZ;
        }
    //    [[nodiscard]] static NiMatrix3 SetRotationMatrixByVector(float Ax, float Ay, float Az, float Bx, float By, float Bz) {
    //        const float cosAxBx = std::cos(Bx);
    //        const float cosAxBy = std::cos(angleRadX);
    //        const float cosAxBz = std::cos(angleRadY);
    //        const float cosAyBx = std::cos(angleRadY);
    //        const float cosAyBy = std::cos(By);
    //        const float cosAyBz = std::cos(angleRadZ);
    //        const float cosAzBx = std::cos(angleRadY);
    //        const float cosAzBy = std::cos(angleRadZ);
    //        const float cosAzBz = std::cos(Bz);
//
    //        NiMatrix3 matrix;
    //        // X axis
    //        matrix.entry[0][0] = 1.0;
    //        matrix.entry[1][0] = cosX;
    //        matrix.entry[2][0] = -sinX;
//
    //        // Y axis
    //        matrix.entry[0][1] = cosY;
    //        matrix.entry[1][1] = sinY;
    //        matrix.entry[2][1] = 1.0;
//
    //        // Z axis
    //        matrix.entry[0][2] = sinZ;
    //        matrix.entry[1][2] = cosZ;
    //        matrix.entry[2][2] = 1.0;
//
    //        return matrix;
    //    }
    };
}
namespace ObjectUtil
{
    struct Node
    {
        static float GetLength(RE::NiAVObject* a_node)
        {
            float length = 0.f;
            if (a_node) {
                length = a_node->worldBound.radius;
            } return length;
        }

	    static RE::NiTransform GetLocalTransform(RE::NiAVObject* a_node, const RE::NiTransform& a_worldTransform, bool a_bUseOldParentTransform = false)    //  from ersh precision
	    {
	    	RE::NiPointer<RE::NiNode> parent(a_node->parent);
	    	if (parent) {
	    		RE::NiTransform inverseParent = (a_bUseOldParentTransform ? parent->previousWorld : parent->world).Invert();
	    		return inverseParent * a_worldTransform;
	    	}
	    	return a_worldTransform;
	    }
    };

    struct Projectile
    {
        static bool DeleteAnExtraArrow(RE::TESObjectREFR* a_victim, RE::NiAVObject* a_arrow3D)
        {
            bool result = false;
            if (a_victim && a_arrow3D) {
            //    auto attachedArrows = static_cast<RE::ExtraAttachedArrows3D*>(a_victim->extraList.GetByType(RE::ExtraDataType::kAttachedArrows3D));
            //    for (auto& extraArrow : attachedArrows->data) {
            //        if (extraArrow.arrow3D.get() == a_arrow3D) {extraArrow.timeStamp = (uint64_t)0; return true;}
            //    }
                if (auto* xList = &a_victim->extraList; xList) 
                    if (auto xArrows = xList->GetByType<RE::ExtraAttachedArrows3D>(); xArrows)
                        if (!xArrows->data.empty())
                            for (auto& extraArrow : xArrows->data) {
                                if (extraArrow.arrow3D.get() && extraArrow.arrow3D.get() == a_arrow3D) {
                                    extraArrow.timeStamp = 0;
                                    extraArrow.arrow3D = nullptr;
                                    result = true;
                                }
                            }
            } return result;
        }
        static bool DeleteAnExtraArrow(RE::TESObjectREFR* a_victim, RE::BSFixedString& a_arrow3Dname)
        {
            bool result = false;
            if (a_victim) {
                if (auto* xList = &a_victim->extraList; xList) 
                    if (auto xArrows = xList->GetByType<RE::ExtraAttachedArrows3D>(); xArrows)
                        if (!xArrows->data.empty())
                            for (auto& extraArrow : xArrows->data) {
                                if (extraArrow.arrow3D.get() && extraArrow.arrow3D.get()->name == a_arrow3Dname) {extraArrow.timeStamp = 0; result = true;}
                            }
            } return result;
        }
    };

    struct Actor
    {
        static void PushActorAway(RE::Actor* a_target, float a_force, RE::NiPoint3 a_direction = RE::NiPoint3()) {
            if (a_target && !a_target->IsDead() && a_target->Is3DLoaded()) {
                auto process = a_target->GetActorRuntimeData().currentProcess;
                if (process && process->InHighProcess()) {
                    return PushActorAwayImpl(process, a_target, a_direction, a_force);
                }
            }
        }
        static void PushActorAwayImpl(RE::AIProcess* a_AIprocess, RE::Actor* a_target, RE::NiPoint3 a_direction, float a_force) 
        {
            using func_t = decltype(&PushActorAwayImpl);
            REL::Relocation<func_t> func{ RELOCATION_ID(38858, 39895) };
            return func(a_AIprocess, a_target, a_direction, a_force);
        }
        /*
        *  Requires my modder utility for this function: https://github.com/PhiloSocio/SkipEquipAnimation
        */
        static void SkipEquipAnimation(RE::Actor* a_this, const bool a_skip = true, int a_load3dDelayMS = 0, const bool a_skip3DLoading = false)
        {
            if (a_this) {
                a_this->SetGraphVariableBool("SkipEquipAnimation", a_skip);
                a_this->SetGraphVariableInt("LoadBoundObjectDelay", a_load3dDelayMS);
                a_this->SetGraphVariableBool("Skip3DLoading", a_skip3DLoading);
            }
        }
        static void SendAnimationEvent(RE::Actor* a_this, const RE::BSFixedString a_tag, const RE::BSFixedString a_payload = "")
        {
            if (a_this) {
                RE::BSAnimationGraphManagerPtr graphManager;

                a_this->GetAnimationGraphManager(graphManager);

                if (graphManager) {
                    bool bSinked = false;
                    for (auto& animationGraph : graphManager->graphs) {
                        if (auto eventSource = animationGraph->GetEventSource<RE::BSAnimationGraphEvent>(); eventSource) {
                            RE::BSAnimationGraphEvent event = {a_tag, a_this, a_payload};
                            a_this->ProcessEvent(&event, eventSource);
                            break;
                        }
                    }
                }
            }
        }
        static bool DoAction(RE::BGSAction* a_action, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton())
        {
            if (auto taskInterface = SKSE::GetTaskInterface(); taskInterface && a_action && a_actor) {
                taskInterface->AddTask([a_action, a_actor]() {
                    std::unique_ptr<TESActionData> data(TESActionData::Create());
                    if (data) {
                        //data->source = a_actor->As<TESObjectREFR>()->GetHandle().get();   //  alternate
                        data->source = NiPointer<TESObjectREFR>(a_actor);
                        data->action = a_action;
                        typedef bool func_t(TESActionData*);
                        REL::Relocation<func_t> func{ RELOCATION_ID(40551, 41557) };        //  credits to https://github.com/jarari
                        return func(data.get());
                    } return false;
                });
            } return false;
        }
        static RE::InventoryEntryData* GetInventoryEntryDataForBoundObject(RE::Actor* a_actor, RE::TESBoundObject* a_boundObj)
        {
            if (a_actor) {
                const auto invChanges = a_actor->GetInventoryChanges();
                RE::BSSimpleList<RE::InventoryEntryData *> *entries = nullptr;
                if (invChanges) entries = invChanges->entryList;
                if (entries && !entries->empty())
                    for (auto entry : *entries) {
                        if (entry && entry->object && entry->object->IsBoundObject() && entry->object == a_boundObj) {
                            return entry;
                        }
                    }
            } return nullptr;
        }
        static void EquipInventoryItem(RE::Actor* a_actor, RE::FormID a_formID, const bool a_skipAnim = false,
         uint32_t a_count = 1U, bool a_queueEquip = true, bool a_forceEquip = false, bool a_playSounds = true, bool a_applyNow = false,
         const RE::BGSEquipSlot *a_slot = (const RE::BGSEquipSlot *)nullptr, const bool a_skip3D = false) {
            if (a_actor) {
                SkipEquipAnimation(a_actor, a_skipAnim, 0, a_skip3D);
                auto eqManager = RE::ActorEquipManager::GetSingleton();
                auto invChanges = a_actor->GetInventoryChanges();
                auto entries = invChanges ? invChanges->entryList : nullptr;
                RE::ExtraDataList* xList = nullptr;
                if (entries)
                    for (auto entry : *entries) {
                        if (entry && eqManager && entry->extraLists && entry->object && entry->object->formID == a_formID) {
                            if (entry->extraLists->empty()) spdlog::warn("your bound object's extralist is empty!");
                            else xList = entry->extraLists->front();
                            eqManager->EquipObject(a_actor, entry->object, xList, a_count, a_slot, a_queueEquip, a_forceEquip, a_playSounds, a_applyNow);
                                break;
                        }
                    }
                else spdlog::warn("there is no inventory changes!");
            }
        }
        static void EquipItem(RE::Actor* a_actor, RE::TESBoundObject* a_boundObject, const bool a_skipAnim = false,
         uint32_t a_count = 1U, bool a_queueEquip = true, bool a_forceEquip = false, bool a_playSounds = true, bool a_applyNow = false,
         const RE::BGSEquipSlot *a_slot = (const RE::BGSEquipSlot *)nullptr, const bool a_skip3D = false) {
            if (a_actor) {
                SkipEquipAnimation(a_actor, a_skipAnim, 0, a_skip3D);
                auto eqManager = RE::ActorEquipManager::GetSingleton();
            //    auto invChanges = a_actor->GetInventoryChanges();
            //    auto entries = invChanges ? invChanges->entryList : nullptr;
            //    RE::ExtraDataList* xList = nullptr;
            //    if (entries)
            //        for (auto entry : *entries) {
            //            if (entry && eqManager && entry->extraLists) {
            //                if (entry->extraLists->empty()) spdlog::warn("your bound object's extralist is empty!");
            //                else xList = entry->extraLists->front();
            //                    break;
            //            }
            //        }
            //    else spdlog::warn("there is no inventory changes!");
                eqManager->EquipObject(a_actor, a_boundObject, nullptr, a_count, a_slot, a_queueEquip, a_forceEquip, a_playSounds, a_applyNow);
            }
        }
        static void UnEquipItem(RE::Actor* a_actor, const bool a_isLeft, const bool a_soundPlay, const bool a_forced = false, const bool a_immediately = false,
         const bool a_skipAnim = false, const bool a_skip3D = false)
        {
            if (a_actor) {
                SkipEquipAnimation(a_actor, a_skipAnim, 0, a_skip3D);
                auto eData = a_actor->GetEquippedEntryData(a_isLeft);
                auto xLists = eData ? eData->extraLists : nullptr;
                auto xList = xLists ? xLists->front() : nullptr;
                auto obj = a_actor->GetEquippedObject(a_isLeft);
                auto boundObj = obj ? obj->As<RE::TESBoundObject>() : nullptr;
                auto eqManager = RE::ActorEquipManager::GetSingleton();
                if (boundObj && eqManager)
                    eqManager->UnequipObject(a_actor, boundObj, xList, 1u, nullptr, !a_immediately, a_forced, a_soundPlay, a_immediately, nullptr);
            }
        }
        static void CastSpell(RE::SpellItem* a_spell, RE::Actor* a_actor, RE::TESObjectREFR *a_target, RE::Actor *a_blameActor, 
         float a_magnitudeOverride = -1.f, const bool a_noHitEffectArt = false, const float a_effectiveness = 1.f, const bool a_hostileEffectivenessOnly = false)
        {
            if (a_actor && a_spell) {
                if (a_spell->effects[0] && a_magnitudeOverride < 0.f) a_magnitudeOverride = a_spell->effects[0]->GetMagnitude();
                if (auto mCaster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant); mCaster) {
                    mCaster->CastSpellImmediate(a_spell, a_noHitEffectArt, a_target, a_effectiveness, a_hostileEffectivenessOnly, a_magnitudeOverride, a_blameActor);
                }
            }
        }
        static float GetAttackDamage(RE::Actor* a_attacker, RE::Actor* a_target)
        {
            float damage = 0.f;
            if (a_attacker && a_target && a_attacker->GetAttackingWeapon()) {
                RE::HitData hitData;
                hitData.Populate(a_attacker, a_target, a_attacker->GetAttackingWeapon());
                damage = hitData.totalDamage;
            } return damage;
        }
        static float GetWeaponAttackDamage(RE::Actor* a_attacker, RE::Actor* a_target, RE::TESObjectWEAP* a_weapon)
        {
            float damage = 0.f;
            if (a_attacker && a_target && a_weapon) {
                if (auto weaponIE = GetInventoryEntryDataForBoundObject(a_attacker, a_weapon); weaponIE) {
                    spdlog::debug("before crash 3");
                    RE::HitData hitData;
                    hitData.Populate(a_attacker, a_target, weaponIE);
                    damage = hitData.totalDamage;
                }
            } return damage;
        }
        template <typename T>
        static T GetNearCombatTargets(RE::Actor* a_this, const float a_distance, const bool a_orderFromNearest = false) {
            T nearTargets; nearTargets;
            if (a_this && a_this->IsInCombat()) {
                auto allyCombatGroup = a_this->GetCombatGroup();
                if (!allyCombatGroup) return nearTargets;
                auto enemyCombatGroupArray = allyCombatGroup->targets;
                nearTargets.reserve(enemyCombatGroupArray.size());
                for (auto& enemyCombatGroup : enemyCombatGroupArray) {
                    if (auto target = enemyCombatGroup.targetHandle.get().get(); target) {
                        if (target->GetPosition().GetDistance(a_this->GetPosition()) <= a_distance) {
                            nearTargets.emplace_back(target);
                        }
                    }
                }
                if (a_orderFromNearest) {
                    std::sort(nearTargets.begin(), nearTargets.end(), 
                        [&](const auto& a, const auto& b) {
                            return a->GetPosition().GetDistance(a_this->GetPosition()) < 
                                   b->GetPosition().GetDistance(a_this->GetPosition());
                        }
                    );
                }
            }
            return nearTargets;
        }
    };

    struct Spell
    {
        static void SetMagnitude(RE::SpellItem* a_spell, const float a_magnitude) 
        {
            if (a_spell && a_spell->effects[0])
                a_spell->effects[0]->effectItem.magnitude = a_magnitude;
        }
        static void SetDuration(RE::SpellItem* a_spell, const uint8_t a_duration) 
        {
            if (a_spell && a_spell->effects[0])
                a_spell->effects[0]->effectItem.duration = a_duration;
        }
    };

    struct Poison
    {
        static RE::AlchemyItem* GetEquippedObjPoison(RE::Actor* a_actor, const bool a_isLeft = false)
        {
            if (a_actor) {
                if (auto eData = a_actor->GetEquippedEntryData(a_isLeft); eData) {
                    auto xList = eData->IsPoisoned() ? eData->extraLists : nullptr;
                    if (xList) return GetPoison(xList);
                }
            } return nullptr;
        }
        static RE::AlchemyItem* GetPoison(RE::BSSimpleList<RE::ExtraDataList *>* a_xList)
        {
            if (a_xList && !a_xList->empty()) {
                for (auto xData : *a_xList) {
                    if (xData)
                        if (auto xPoison = xData->GetByType<RE::ExtraPoison>(); auto poison = xPoison ? xPoison->poison : nullptr) {
                            return poison;
                        }
                }
            } return nullptr;
        }
    };

    struct Enchantment
    {
        //  setters
        static void EnchantEquippedWeapon(RE::Actor* a_actor, RE::EnchantmentItem* a_ench, const float a_charge = 500.f, const bool a_isLeft = false, const bool a_removeOnUnequip = false)
        {
            if (!a_actor || !a_ench) return;
            if (auto eData = a_actor->GetEquippedEntryData(a_isLeft); auto xList = eData ? eData->extraLists : nullptr) {
                if (xList) return EnchantItem(xList, a_ench, a_charge, a_removeOnUnequip);
            }
        }
        static void DisEnchantEquippedWeapon(RE::Actor* a_actor, const bool a_isLeft = false, const bool a_defaultEnch = false)
        {
            if (auto eData = a_actor->GetEquippedEntryData(a_isLeft); auto obj = a_actor->GetEquippedObject(a_isLeft)) {
                if (auto weap = obj->As<RE::TESObjectWEAP>(); weap) {
                    if (a_defaultEnch && weap->formEnchanting) {
                        weap->formEnchanting = nullptr;
                        weap->amountofEnchantment = 0;
                    }
                    if (auto xList = eData->extraLists; xList) {
                        return DisEnchantItem(xList);
                    }
                }
            }
        }
        static void ChargeEquippedWeapon(RE::Actor* a_actor, const float a_charge, const bool a_isLeft = false)
        {
            if (a_actor && a_actor->AsActorValueOwner()) {
                float maxCharge = 0.f;
                if (auto eData = a_actor->GetEquippedEntryData(a_isLeft); auto xList = eData ? eData->extraLists : nullptr) {
                    if (eData->object && eData->object->As<RE::TESObjectWEAP>()) {
                        maxCharge = eData->object->As<RE::TESObjectWEAP>()->amountofEnchantment;
                    } for (auto xData : *xList) {
                        if (auto xEnch = xData->GetByType<RE::ExtraEnchantment>(); xEnch) {
                            maxCharge = (float)xEnch->charge - 1.f;
                        }
                        if (auto xCharge = xData->GetByType<RE::ExtraCharge>(); xCharge) {
                            float sum = xCharge->charge + a_charge;
                            if (sum < 0.f) sum = 0.f;
                            xCharge->charge = sum >= maxCharge ? maxCharge : sum;
                                break;
                        }
                    }
                }
                const auto av = a_isLeft ? RE::ActorValue::kLeftItemCharge : RE::ActorValue::kRightItemCharge;
                const float sum = a_charge + a_actor->AsActorValueOwner()->GetActorValue(av);
                const float charge = sum >= maxCharge ? maxCharge : sum;
                a_actor->AsActorValueOwner()->ModActorValue(av, charge);
            }
        }
        static void ChargeInventoryWeapon(RE::Actor* a_actor, RE::TESBoundObject* a_weap, const uint16_t  a_charge)
        {
            if (a_actor) {
                auto invChanges = a_actor->GetInventoryChanges();
                if (auto entries = invChanges ? invChanges->entryList : nullptr; entries && !entries->empty())
                    for (auto entry : *entries) {
                        if (entry && entry->object && entry->object->IsWeapon() && entry->object == a_weap) {
                            ChargeWeapon(entry, a_charge);
                        }
                    }
            }
        }
        static void ChargeWeapon(RE::InventoryEntryData* a_eData, const uint16_t  a_charge)
        {
            if (a_eData && a_eData->extraLists && !a_eData->extraLists->empty()) {
                uint16_t maxCharge = 0u;
                auto xList = a_eData->extraLists;
                if (a_eData->object && a_eData->object->As<RE::TESObjectWEAP>()) {
                    maxCharge = a_eData->object->As<RE::TESObjectWEAP>()->amountofEnchantment;
                } for (auto xData : *xList) {
                    if (xData) {
                        if (auto xEnch = xData->GetByType<RE::ExtraEnchantment>(); xEnch) {
                            maxCharge = xEnch->charge;
                        }
                        if (auto xCharge = xData->GetByType<RE::ExtraCharge>(); xCharge) {
                            float sum = xCharge->charge + a_charge;
                            if (sum < 0.f) sum = 0.f;
                            xCharge->charge = sum > maxCharge ? maxCharge : sum;
                                break;
                        }
                    }
                }
            }
        }
        
        static void EnchantInventoryWeapon(RE::Actor* a_actor, RE::FormID a_weapID, RE::EnchantmentItem* a_ench, const float a_magnitude, const float a_charge, const bool a_removeOnUnequip = false)
        {
            if (!a_actor || !a_ench) return;

            auto invChanges = a_actor->GetInventoryChanges();
            auto entries = invChanges ? invChanges->entryList : nullptr;
            RE::BSSimpleList<RE::ExtraDataList *>* xList = nullptr;
            if (entries && !entries->empty())
                for (auto entry : *entries) {
                    if (entry && entry->object && entry->object->IsWeapon() && entry->object->formID == a_weapID) {
                        xList = entry->extraLists;
                    }
                } if (xList) return EnchantItem(xList, a_ench, a_charge, a_removeOnUnequip);
        }
        static void EnchantItem(RE::BSSimpleList<RE::ExtraDataList *>* a_xList, RE::EnchantmentItem* a_ench, const float a_charge = 500.f, const bool a_removeOnUnequip = false)
        {
            if (!a_xList || a_xList->empty() || !a_ench) return;

            bool isEnchanted = false;
            bool isCharged = false;
            for (auto xData : *a_xList) {
                if (xData) {
                    if (auto xEnch = xData->GetByType<RE::ExtraEnchantment>(); xEnch) {
                        xEnch->enchantment = a_ench;
                        xEnch->charge = a_charge;
                        xEnch->removeOnUnequip = a_removeOnUnequip;
                        isEnchanted = true;
                    } if (auto xCharge = xData->GetByType<RE::ExtraCharge>(); xCharge) {
                        xCharge->charge = a_charge;
                        isCharged = true;
                    }
                }
            }
            if (!isEnchanted)
                for (auto xData : *a_xList) {
                    auto newEnch = RE::BSExtraData::Create<RE::ExtraEnchantment>();
                    newEnch->enchantment = a_ench;
                    newEnch->charge = a_charge;
                    newEnch->removeOnUnequip = a_removeOnUnequip;
                //    RE::ExtraEnchantment* newEnch = new RE::ExtraEnchantment(a_ench, a_charge, a_removeOnUnequip);
                //    RE::ExtraEnchantment newEnch(a_ench, a_charge, a_removeOnUnequip);    //  causing crashes
                    if (xData) {
                        xData->Add(newEnch);
                        break;
                    }
                }
            if (!isCharged)
                for (auto xData : *a_xList) {
                //    RE::ExtraCharge* newCharge = new RE::ExtraCharge();
                    RE::ExtraCharge* newCharge = RE::BSExtraData::Create<RE::ExtraCharge>();
                    newCharge->charge = a_charge;
                    if (xData) {
                        xData->Add(newCharge);
                        break;
                    }
                }
        }
        static void DisEnchantInventoryWeapon(RE::Actor* a_actor, RE::TESBoundObject* a_boundObj)
        {
            if (a_actor) {
                auto invChanges = a_actor->GetInventoryChanges();
                auto entries = invChanges->entryList;
                RE::BSSimpleList<RE::ExtraDataList *>* xList = nullptr;
                for (auto entry : *entries) {
                    if (entry && entry->object && entry->object->IsWeapon() && entry->object == a_boundObj) {
                        xList = entry->extraLists;
                    }
                } if (xList) return DisEnchantItem(xList);
            }
        }
        static void DisEnchantItem(RE::BSSimpleList<RE::ExtraDataList *>* a_xList)
        {
            if (a_xList) {
                for (auto xData : *a_xList) {
                    if (xData) {
                        if (auto xEnch = xData->GetByType<RE::ExtraEnchantment>(); xEnch) {
                            xEnch->enchantment = nullptr;
                            xEnch->charge = 0;
                            xData->Remove(xEnch);
                            xEnch->~ExtraEnchantment();
                            xEnch = nullptr;
                        } if (auto xCharge = xData->GetByType<RE::ExtraCharge>(); xCharge) {
                            xCharge->charge = 0.f;
                            xData->Remove(xCharge);
                            xCharge->~ExtraCharge();
                            xCharge = nullptr;
                        }
                    }
                }
            }
        }
        //  getters
        static RE::EnchantmentItem* GetEquippedWeaponEnchantment(RE::Actor* a_actor, const bool a_isLeft = false, const bool a_baseEnchPrior = false)
        {
            RE::EnchantmentItem* ench = nullptr;
            if (a_actor)
                if (auto obj = a_actor->GetEquippedObject(a_isLeft); obj && obj->As<RE::TESBoundObject>()) ench = GetInventoryItemEnchantment(a_actor, obj->As<RE::TESBoundObject>(), a_baseEnchPrior);
            return ench;
        }
        static float GetEquippedWeaponCharge(RE::Actor* a_actor, const bool a_isLeft = false)
        {
            float charge = 0.f;
            if (a_actor && a_actor->AsActorValueOwner()) {
                const auto av = a_isLeft ? RE::ActorValue::kLeftItemCharge : RE::ActorValue::kRightItemCharge;
                charge = a_actor->AsActorValueOwner()->GetActorValue(av);
            } return charge;
        }
        static RE::EnchantmentItem* GetInventoryItemEnchantment(RE::Actor* a_actor, RE::TESBoundObject* a_boundObj, const bool a_baseEnchPrior = false)
        {
            RE::EnchantmentItem* formEnch = nullptr;
            RE::EnchantmentItem* ench = nullptr;
            if (a_actor) {
                auto invChanges = a_actor->GetInventoryChanges();
                auto entries = invChanges->entryList;
                for (auto entry : *entries) {
                    if (entry && entry->object && entry->object->IsWeapon() && entry->object == a_boundObj) {
                        if (auto eForm = entry->object->As<RE::TESEnchantableForm>(); eForm) formEnch = eForm->formEnchanting;
                        if (auto xList = entry->extraLists; xList && !xList->empty()) ench = GetExtraEnchantment(xList);
                    }
                }
            }
            if (a_baseEnchPrior) return formEnch ? formEnch : ench;
            else return ench ? ench : formEnch;
        }
        static RE::EnchantmentItem* GetExtraEnchantment(RE::BSSimpleList<RE::ExtraDataList *>* a_xList)
        {
            if (a_xList && !a_xList->empty()) {
                for (auto xData : *a_xList) {
                    if (xData)
                        if (auto xEnch = xData->GetByType<RE::ExtraEnchantment>(); xEnch)
                            return xEnch->enchantment;
                }
            } return nullptr;
        }
    };

    struct Sound
    {
        static void PlaySound(RE::BGSSoundDescriptorForm* a_sound, RE::NiAVObject* a_source, const float a_volume = 1.f, RE::BSSoundHandle* a_handle = nullptr)
        {
            if (a_sound && a_source) {
                auto audioManager = RE::BSAudioManager::GetSingleton();
                RE::BSSoundHandle handle;
                RE::BSSoundHandle& refHandle = a_handle ? *a_handle : handle;
                audioManager->BuildSoundDataFromDescriptor(refHandle, a_sound->soundDescriptor);
                refHandle.SetObjectToFollow(a_source);
                refHandle.SetVolume(a_volume);
                refHandle.Play();
            }
        }
    };

    struct Transform
    {
        static void TranslateTo(RE::BSScript::IVirtualMachine *vm, RE::VMStackID stackID, RE::TESObjectREFR *object, float afX, float afY, float afZ, float afAngleX, float afAngleY, float afAngleZ, float afSpeed, float afMaxRotationSpeed)
        {
            using func_t = decltype(TranslateTo);
            REL::Relocation<func_t> func{RELOCATION_ID(55706, 56237)};
            func(vm, stackID, object, afX, afY, afZ, afAngleX, afAngleY, afAngleZ, afSpeed, afMaxRotationSpeed);
        }

        static float InterpAngleTo(float a_current, float a_target, float a_deltaTime, float a_interpSpeed)
        {
            if (a_interpSpeed <= 0.f)
            {
                return a_target;
            }

            const float distance = MathUtil::Angle::NormalRelativeAngle(a_target - a_current);

            if (distance * distance < FLT_EPSILON)
            {
                return a_target;
            }

            const float delta = distance * Clamp(a_deltaTime * a_interpSpeed, 0.f, 1.f);

            return a_current + delta;
        }

        static float Clamp(float value, float min, float max)
        {
            return value < min ? min : value < max ? value
                                                   : max;
        }
    };
}


namespace AnimUtil
{
    struct Idle
    {
            static bool Play(RE::TESIdleForm* idle, RE::Actor* actor, RE::DEFAULT_OBJECT action, RE::Actor* target)
            {
            if (actor && actor->GetActorRuntimeData().currentProcess)
            {
                typedef bool (*func_t)(RE::AIProcess *, RE::Actor *, RE::DEFAULT_OBJECT, RE::TESIdleForm *, bool, bool, RE::Actor *);
                REL::Relocation<func_t> func{RELOCATION_ID(38290, 39256)};
                return func(actor->GetActorRuntimeData().currentProcess, actor, action, idle, true, true, target);
            }
            return false;
            }
    };
}

namespace FormUtil
{
    struct Parse
    {
            static RE::TESForm *GetFormFromMod(uint32_t formid,std::string modname)
            {
            if (!modname.length() || !formid)
                return nullptr;
            RE::TESDataHandler *dh = RE::TESDataHandler::GetSingleton();
            return dh->LookupForm(formid, modname); 

            }

            static RE::TESForm *GetFormFromMod(std::string modname, std::string formIDString)
            {
                if (formIDString.length() == 0) return nullptr; 

                uint32_t formID = std::stoi(formIDString, 0, 16); 
                return GetFormFromMod(formID,modname); 
            } 

        //    static RE::TESForm *GetFormFromConfigString(std::string str, std::string_view delimiter)
        //    {
        //        std::vector<std::string> splitData = Util::String::Split(str, delimiter); 
        //        if (splitData.size() < 2) return nullptr;  
        //        return GetFormFromMod(splitData[1], splitData[0]);
        //    }
        //    static RE::TESForm *GetFormFromConfigString(std::string str)
        //    {
        //        return GetFormFromConfigString(str, "~"sv); 
        //    }
            static RE::FormID GetFormIDFromMod(uint32_t relativeFormID, std::string modName)
            {
                auto *dataHandler = TESDataHandler::GetSingleton();

                if (!dataHandler)
                return -1;

                return dataHandler->LookupFormID(relativeFormID, modName);
            }

            static RE::FormID GetFormIDFromMod(std::string relativeFormIDString, std::string modName)
            {
                if (relativeFormIDString.length() == 0) return -1; 


                uint32_t relativeFormID = std::stoi(relativeFormIDString,  0, 16); 
                return GetFormIDFromMod(relativeFormID, modName); 
            }

        //    static RE::FormID GetFormIDFromConfigString(std::string str, std::string_view delimiter)
        //    {
        //        std::vector<std::string> splitData = Util::String::Split(str, delimiter); 
        //        if (splitData.size() < 2) return -1; 
        //        return GetFormIDFromMod(splitData[0], splitData[1]);
        //    }
        //    static RE::FormID GetFormIDFromConfigString(std::string str)
        //    {
        //        return GetFormIDFromConfigString(str, "~"sv); 
        //    }

    };

    struct Quest 
    {
        public:
            static BGSBaseAlias *FindAliasByName(std::string_view name, TESQuest *owningQuest)
            {
                RE::BSWriteLockGuard AliasLock{owningQuest->aliasAccessLock};
                for (auto *alias : owningQuest->aliases)
                {
                std::string aliasName = alias->aliasName.c_str();
                if (aliasName == name)
                    return alias;
                }
                return nullptr;
            }
    };
}
namespace NifUtil
{
    struct Node
        {
            static NiAVObject* Clone(NiAVObject* original)
            {
                typedef NiAVObject* (*func_t)(NiAVObject* avObj);
                REL::Relocation<func_t> func{ RELOCATION_ID(68835, 70187) };
                return func(original);
            }
            static RE::NiAVObject* GetNiObject(
                RE::NiNode*              a_root,
                const RE::BSFixedString& a_name)
            {
                return a_root->GetObjectByName(a_name);
            }

            static void AttachToNode(
                RE::NiAVObject* a_object,
                RE::NiNode*     a_node)
            {
                if (a_object->parent != a_node)
                {
                    a_node->AttachChild(a_object, true);
                }
            }

            static std::vector<BSGeometry*> GetAllGeometries(RE::NiAVObject* root)
            {
                std::vector<BSGeometry*> geometries; 
                RE::BSVisit::TraverseScenegraphGeometries(root, [&](BSGeometry* geom)-> RE::BSVisit::BSVisitControl 
                {
                    geometries.emplace_back(geom); 
                    return RE::BSVisit::BSVisitControl::kContinue;
                }
                ); 
                return geometries;
            }

        };
    struct Armature
    {
        static RE::NiNode* GetActorNode(RE::Actor* actor, std::string nodeName)
        {
                auto root = actor->Get3D();
                if (!root) return nullptr;

                auto bone = root->GetObjectByName(nodeName);
                if (!bone) return nullptr;

                auto node = bone->AsNode();
                if (!node) return nullptr;

                return node;
        }

        static void AttachToNode(RE::NiAVObject* obj, RE::Actor* actor, std::string nodeName)
        {
            auto* node = GetActorNode(actor, nodeName);
            if (node)
            {
                node->AttachChild(obj, true);
                SKSE::log::info("Object Attached");
            }
        }
    };
    struct Collision
    {
        
        [[nodiscard]] static RE::bhkRigidBody* GetRigidBody(RE::NiAVObject* a_object)
        {
            auto collisionObject = a_object->GetCollisionObject();
            if (collisionObject) {
                return collisionObject->GetRigidBody();
            }
            return nullptr;
        }
        static bool ToggleMeshCollision(RE::NiAVObject* root,RE::bhkWorld* world, bool collisionState)
        {
            constexpr auto no_collision_flag = static_cast<std::uint32_t>(RE::CFilter::Flag::kNoCollision);
                    if (root && world) {
                        
                            RE::BSWriteLockGuard locker(world->worldLock);

                            RE::BSVisit::TraverseScenegraphCollision(root, [&](RE::bhkNiCollisionObject* a_col) -> RE::BSVisit::BSVisitControl {
                                if (auto hkpBody = a_col->body ? static_cast<RE::hkpWorldObject*>(a_col->body->referencedObject.get()) : nullptr; hkpBody) {
                                    auto& filter = hkpBody->collidable.broadPhaseHandle.collisionFilterInfo;
                                    if (!collisionState) {
                                        filter |= no_collision_flag;
                                    } else {
                                        filter &= ~no_collision_flag;
                                    }
                                }
                                return RE::BSVisit::BSVisitControl::kContinue;
                            });
                    }
                    else 
                    {
                        return false;
                    }
            return true;
        }
        static bool RemoveMeshCollision(RE::NiAVObject* root,RE::bhkWorld* world, bool collisionState)
        {
            constexpr auto no_collision_flag = static_cast<std::uint32_t>(RE::CFilter::Flag::kNoCollision);
                    if (root && world) {
                        
                            RE::BSWriteLockGuard locker(world->worldLock);

                            RE::BSVisit::TraverseScenegraphCollision(root, [&](RE::bhkNiCollisionObject* a_col) -> RE::BSVisit::BSVisitControl {
                                if (auto hkpBody = a_col->body ? static_cast<RE::hkpWorldObject*>(a_col->body->referencedObject.get()) : nullptr; hkpBody) {
                                    auto& filter = hkpBody->collidable.broadPhaseHandle.collisionFilterInfo;
                                    if (!collisionState) {
                                        filter |= no_collision_flag;
                                    } else {
                                        filter &= ~no_collision_flag;
                                    }
                                }
                                return RE::BSVisit::BSVisitControl::kContinue;
                            });
                    }
                    else 
                    {
                        return false;
                    }
            return true;
        }
    };
}