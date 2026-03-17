#pragma once

#include <Flux/Core/Types.hpp>

namespace llm_studio {

namespace Theme {
    constexpr flux::Color Background    = {0.08f, 0.08f, 0.08f};
    constexpr flux::Color Surface       = {0.12f, 0.12f, 0.12f};
    constexpr flux::Color SurfaceRaised = {0.16f, 0.16f, 0.16f};
    constexpr flux::Color Border        = {0.22f, 0.22f, 0.22f};
    constexpr flux::Color TextPrimary   = {0.92f, 0.92f, 0.92f};
    constexpr flux::Color TextMuted     = {0.48f, 0.48f, 0.48f};
    constexpr flux::Color Accent        = {0.38f, 0.57f, 1.00f};
    constexpr flux::Color AccentHover   = {0.50f, 0.67f, 1.00f};
    constexpr flux::Color Destructive   = {0.90f, 0.30f, 0.30f};
    constexpr flux::Color Success       = {0.35f, 0.75f, 0.50f};

    constexpr float FontH1       = 18.0f;
    constexpr float FontH2       = 14.0f;
    constexpr float FontBody     = 14.0f;
    constexpr float FontCaption  = 12.0f;
    constexpr float FontCode     = 13.0f;
    constexpr float FontBadge    = 11.0f;

    constexpr float RadiusSmall  = 4.0f;
    constexpr float RadiusCard   = 8.0f;
    constexpr float RadiusDialog = 10.0f;
    constexpr float RadiusPill   = 18.0f;

    constexpr float Space1  = 4.0f;
    constexpr float Space2  = 8.0f;
    constexpr float Space3  = 12.0f;
    constexpr float Space4  = 16.0f;
    constexpr float Space6  = 24.0f;
    constexpr float Space8  = 32.0f;
    constexpr float Space12 = 48.0f;
}

} // namespace llm_studio
