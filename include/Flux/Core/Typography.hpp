#pragma once

#include <Flux/Core/Types.hpp>

namespace flux {

/**
 * macOS / San Francisco–aligned typography defaults (pt semantics in Flux = logical px).
 * @see Apple Human Interface Guidelines — Typography
 */
namespace Typography {

// --- Type sizes (pt) ---------------------------------------------------------
inline constexpr float body = 17.0f;           // Primary reading & UI text
inline constexpr float subheadline = 14.0f;      // Secondary labels, section headers
inline constexpr float callout = 15.0f;         // Control labels, emphasized secondary
/** Single-line inputs: TextInput, SelectInput, DropdownMenu trigger (matches callout size). */
inline constexpr float input = 15.0f;
inline constexpr float caption = 12.0f;          // Minimum functional UI size (avoid going smaller)

// --- Line height multipliers (NanoVG text line height) ----------------------
/** Long-form / body copy; WCAG-friendly (~150% leading). */
inline constexpr float lineHeightBody = 1.5f;
/** Wrapped labels & paragraphs; between 1.35 and 1.5 for screen readability. */
inline constexpr float lineHeightReadable = 1.35f;
/** Single-line UI: headlines, buttons, table cells (tighter leading). */
inline constexpr float lineHeightTight = 1.2f;

/** Extra space after paragraph breaks as a multiple of font size (≈ 2× per HIG). */
inline constexpr float paragraphSpacingFactor = 2.0f;

// --- Tracking (letter spacing, same units as NanoVG / font size) ------------
/** ~−2.5% em equivalent for SF body sizes. */
inline constexpr float trackingBody(float size) {
    return -0.025f * size;
}
/** ~+1.1% for large titles. */
inline constexpr float trackingTitle(float size) {
    return 0.011f * size;
}
/** Small caps / captions: slight positive tracking for legibility at minimum sizes. */
inline constexpr float trackingCaption(float size) {
    return 0.02f * size + 0.25f;
}

/** Pick tracking for arbitrary UI text (weight reserved for future optical tuning). */
inline constexpr float trackingFor(float size, FontWeight /* weight */) {
    if (size >= 22.0f) {
        return trackingTitle(size);
    }
    if (size <= 13.0f) {
        return trackingCaption(size);
    }
    return trackingBody(size);
}

} // namespace Typography

} // namespace flux
