#pragma once

namespace DebugRenderer
{
    bool Install();
    void DrawLine(const RE::NiPoint3& start, const RE::NiPoint3& end, const RE::NiColorA& color);
    void DrawPoint(const RE::NiPoint3& pos, float radius, const RE::NiColorA& color);
    void DrawPlainAxis(const RE::NiPoint3& pos, float size, const RE::NiColorA& color);
}