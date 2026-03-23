#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/Typography.hpp>

namespace flux {

/**
 * Shared metrics for single-line controls (TextInput, SelectInput, DropdownMenu trigger)
 * so they align in a row. Matches TextInput defaults: Typography::input, 8px padding, 36px height.
 */
namespace ControlMetrics {

inline constexpr float kStandardFontSize = Typography::input;
inline constexpr EdgeInsets kStandardPadding = EdgeInsets(8.0f, 8.0f, 8.0f, 8.0f);
inline constexpr float kStandardMinHeight = 36.0f;

/** Menu rows: same vertical padding as control chrome; horizontal inset for label gutters. */
inline constexpr EdgeInsets kMenuRowPadding = EdgeInsets(8.0f, 10.0f, 8.0f, 10.0f);

} // namespace ControlMetrics

} // namespace flux
