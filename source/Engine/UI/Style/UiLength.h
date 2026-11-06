#pragma once
enum class UiLenKind { Px, Percent, Calc, FitChildren, FillParentFlow, HeightPercentage, WidthPercentage, Auto };

struct UiLength
{
    UiLenKind Kind = UiLenKind::Px;
    float Px = 0.f;        // Px, Calc (pixel part)
    float Percent = 0.f;   // Percent, Calc (percent part), Height/WidthPercentage (0..100)
    float Weight = 1.f;    // FillParentFlow
    float Resolve(float basis) const { return Px + Percent * 0.01f * basis; }   // Px, Percent, Calc only
    bool IsResolvable() const { return Kind == UiLenKind::Px || Kind == UiLenKind::Percent || Kind == UiLenKind::Calc; }
    bool HasPercent() const { return (Kind == UiLenKind::Percent || Kind == UiLenKind::Calc) && Percent != 0.f; }
    bool operator==(const UiLength&) const = default;
};

