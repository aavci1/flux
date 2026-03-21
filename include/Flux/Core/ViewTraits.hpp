#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/EventTypes.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <optional>
#include <functional>
#include <concepts>
#include <string>
#include <vector>

namespace flux {

class View;

// Tier 1: Visual + layout essentials (every component gets these).
struct ViewProperties {
    Property<EdgeInsets> padding = {};
    Property<Color> backgroundColor = Colors::transparent;
    Property<BackgroundImage> backgroundImage = BackgroundImage{};
    Property<Color> borderColor = Colors::transparent;
    Property<float> borderWidth = 0;
    Property<CornerRadius> cornerRadius = CornerRadius{0, 0, 0, 0};
    Property<float> opacity = 1.0;
    Property<bool> visible = true;
    Property<bool> clip = false;
    Property<float> expansionBias = 0.0f;
    Property<float> compressionBias = 1.0f;
    Property<std::optional<float>> minWidth = std::nullopt;
    Property<std::optional<float>> maxWidth = std::nullopt;
    Property<std::optional<float>> minHeight = std::nullopt;
    Property<std::optional<float>> maxHeight = std::nullopt;
    Property<int> colspan = 1;
    Property<int> rowspan = 1;
    Property<std::optional<CursorType>> cursor = std::nullopt;
    Property<bool> focusable = false;
    Property<std::string> focusKey = "";
    Property<std::string> key = "";
};

// Tier 2: Interactive components add these event callbacks.
struct InteractiveProperties {
    std::function<void()> onClick = nullptr;
    std::function<void(float, float, int)> onMouseDown = nullptr;
    std::function<void(float, float, int)> onMouseUp = nullptr;
    std::function<void(float, float)> onMouseMove = nullptr;
    std::function<void()> onMouseEnter = nullptr;
    std::function<void()> onMouseLeave = nullptr;
    std::function<void()> onDoubleClick = nullptr;
    std::function<void()> onFocus = nullptr;
    std::function<void()> onBlur = nullptr;
    std::function<bool(const KeyEvent&)> onKeyDown = nullptr;
    std::function<bool(const KeyEvent&)> onKeyUp = nullptr;
    std::function<void(const std::string&)> onTextInput = nullptr;
    std::function<void()> onChange = nullptr;
    std::function<void(float, float, float, float)> onScroll = nullptr;
};

// Tier 3: Transform properties (most components don't need these).
struct TransformProperties {
    Property<float> rotation = 0;
    Property<float> scaleX = 1.0;
    Property<float> scaleY = 1.0;
    Property<Point> offset = Point{0, 0};
};

// Legacy macros — kept for gradual migration. Prefer inheriting from the
// mixin structs above.
#define FLUX_VIEW_PROPERTIES \
    Property<EdgeInsets> padding = {}; \
    Property<Color> backgroundColor = Colors::transparent; \
    Property<BackgroundImage> backgroundImage = BackgroundImage{}; \
    Property<Color> borderColor = Colors::transparent; \
    Property<float> borderWidth = 0; \
    Property<CornerRadius> cornerRadius = CornerRadius{0, 0, 0, 0}; \
    Property<float> opacity = 1.0; \
    Property<bool> visible = true; \
    Property<bool> clip = false; \
    Property<float> expansionBias = 0.0f; \
    Property<float> compressionBias = 1.0f; \
    Property<std::optional<float>> minWidth = std::nullopt; \
    Property<std::optional<float>> maxWidth = std::nullopt; \
    Property<std::optional<float>> minHeight = std::nullopt; \
    Property<std::optional<float>> maxHeight = std::nullopt; \
    Property<int> colspan = 1; \
    Property<int> rowspan = 1; \
    Property<std::optional<CursorType>> cursor = std::nullopt; \
    Property<bool> focusable = false; \
    Property<std::string> focusKey = ""; \
    Property<std::string> key = ""

#define FLUX_INTERACTIVE_PROPERTIES \
    std::function<void()> onClick = nullptr; \
    std::function<void(float, float, int)> onMouseDown = nullptr; \
    std::function<void(float, float, int)> onMouseUp = nullptr; \
    std::function<void(float, float)> onMouseMove = nullptr; \
    std::function<void()> onMouseEnter = nullptr; \
    std::function<void()> onMouseLeave = nullptr; \
    std::function<void()> onDoubleClick = nullptr; \
    std::function<void()> onFocus = nullptr; \
    std::function<void()> onBlur = nullptr; \
    std::function<bool(const KeyEvent&)> onKeyDown = nullptr; \
    std::function<bool(const KeyEvent&)> onKeyUp = nullptr; \
    std::function<void(const std::string&)> onTextInput = nullptr; \
    std::function<void()> onChange = nullptr; \
    std::function<void(float, float, float, float)> onScroll = nullptr

#define FLUX_TRANSFORM_PROPERTIES \
    Property<float> rotation = 0; \
    Property<float> scaleX = 1.0; \
    Property<float> scaleY = 1.0; \
    Property<Point> offset = Point{0, 0}

// Concept for what makes a View component
// All methods are now optional - if not defined, default implementations will be used
template<typename T>
concept ViewComponent =
    !std::is_same_v<std::remove_cvref_t<T>, View> &&
    requires(const T& t) {
        { t.visible } -> std::convertible_to<bool>;
        { t.padding };
    };

// SFINAE helpers to detect if methods exist
template<typename T>
struct has_layout {
    template<typename U>
    static auto test(U&& u) -> decltype(u.layout(std::declval<RenderContext&>(), std::declval<const Rect&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T>()))::value;
};

template<typename T>
struct has_body {
    template<typename U>
    static auto test(const U& u) -> decltype(u.body(), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_render {
    template<typename U>
    static auto test(const U& u) -> decltype(u.render(std::declval<RenderContext&>(), std::declval<const Rect&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_preferredSize {
    template<typename U>
    static auto test(const U& u) -> decltype(u.preferredSize(std::declval<TextMeasurement&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_heightForWidth {
    template<typename U>
    static auto test(const U& u) -> decltype(u.heightForWidth(std::declval<float>(), std::declval<TextMeasurement&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_children_property {
    template<typename U>
    static auto test(const U& u) -> decltype(u.children, std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

#define FLUX_DEFINE_HAS_FIELD(name) \
template<typename T> struct has_##name { \
    template<typename U> static auto test(const U& u) -> decltype(u.name, std::true_type{}); \
    static std::false_type test(...); \
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value; \
};

FLUX_DEFINE_HAS_FIELD(onClick)
FLUX_DEFINE_HAS_FIELD(onMouseDown)
FLUX_DEFINE_HAS_FIELD(onMouseUp)
FLUX_DEFINE_HAS_FIELD(onMouseMove)
FLUX_DEFINE_HAS_FIELD(onMouseEnter)
FLUX_DEFINE_HAS_FIELD(onMouseLeave)
FLUX_DEFINE_HAS_FIELD(onDoubleClick)
FLUX_DEFINE_HAS_FIELD(onFocus)
FLUX_DEFINE_HAS_FIELD(onBlur)
FLUX_DEFINE_HAS_FIELD(onKeyDown)
FLUX_DEFINE_HAS_FIELD(onKeyUp)
FLUX_DEFINE_HAS_FIELD(onTextInput)
FLUX_DEFINE_HAS_FIELD(onChange)
FLUX_DEFINE_HAS_FIELD(onScroll)

#undef FLUX_DEFINE_HAS_FIELD

template<typename T>
struct has_handleKeyDown {
    template<typename U>
    static auto test(U& u) -> decltype(u.handleKeyDown(std::declval<const KeyEvent&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T&>()))::value;
};

template<typename T>
struct has_handleKeyUp {
    template<typename U>
    static auto test(U& u) -> decltype(u.handleKeyUp(std::declval<const KeyEvent&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T&>()))::value;
};

template<typename T>
struct has_handleTextInput {
    template<typename U>
    static auto test(U& u) -> decltype(u.handleTextInput(std::declval<const TextInputEvent&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T&>()))::value;
};

template<typename T>
struct has_capturePointerEvent {
    template<typename U>
    static auto test(U& u) -> decltype(u.capturePointerEvent(std::declval<PointerEvent&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T&>()))::value;
};

template<typename T>
struct has_init {
    template<typename U>
    static auto test(U&& u) -> decltype(u.init(), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T>()))::value;
};

template<typename T>
struct has_onMount {
    template<typename U>
    static auto test(U&& u) -> decltype(u.onMount(), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T>()))::value;
};

template<typename T>
struct has_onUnmount {
    template<typename U>
    static auto test(U&& u) -> decltype(u.onUnmount(), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T>()))::value;
};

// SFINAE traits for testing/accessibility introspection of component properties
template<typename T, typename = void>
struct has_text_string : std::false_type {};
template<typename T>
struct has_text_string<T, std::void_t<
    decltype(std::string{static_cast<std::string>(std::declval<const T&>().text)})
>> : std::true_type {};

template<typename T, typename = void>
struct has_value_string : std::false_type {};
template<typename T>
struct has_value_string<T, std::void_t<
    decltype(std::string{static_cast<std::string>(std::declval<const T&>().value)})
>> : std::true_type {};

template<typename T, typename = void>
struct has_value_float : std::false_type {};
template<typename T>
struct has_value_float<T, std::void_t<
    decltype(static_cast<float>(std::declval<const T&>().value)),
    std::enable_if_t<!has_value_string<T>::value>
>> : std::true_type {};

template<typename T, typename = void>
struct has_label_string : std::false_type {};
template<typename T>
struct has_label_string<T, std::void_t<
    decltype(std::string{static_cast<std::string>(std::declval<const T&>().label)})
>> : std::true_type {};

template<typename T, typename = void>
struct has_checked_bool : std::false_type {};
template<typename T>
struct has_checked_bool<T, std::void_t<
    decltype(static_cast<bool>(std::declval<const T&>().checked))
>> : std::true_type {};

template<typename T, typename = void>
struct has_isOn_bool : std::false_type {};
template<typename T>
struct has_isOn_bool<T, std::void_t<
    decltype(static_cast<bool>(std::declval<const T&>().isOn))
>> : std::true_type {};

template<typename T, typename = void>
struct has_placeholder_string : std::false_type {};
template<typename T>
struct has_placeholder_string<T, std::void_t<
    decltype(std::string{static_cast<std::string>(std::declval<const T&>().placeholder)})
>> : std::true_type {};

template<typename T, typename = void>
struct has_selected_bool : std::false_type {};
template<typename T>
struct has_selected_bool<T, std::void_t<
    decltype(static_cast<bool>(std::declval<const T&>().selected))
>> : std::true_type {};

template<typename T, typename = void>
struct has_fontSize_float : std::false_type {};
template<typename T>
struct has_fontSize_float<T, std::void_t<
    decltype(static_cast<float>(std::declval<const T&>().fontSize))
>> : std::true_type {};

template<typename T, typename = void>
struct has_selection_state : std::false_type {};
template<typename T>
struct has_selection_state<T, std::void_t<
    decltype(std::declval<const T&>().selStart),
    decltype(std::declval<const T&>().selEnd),
    decltype(std::declval<const T&>().caretPos),
    decltype(std::string{static_cast<std::string>(std::declval<const T&>().value)})
>> : std::true_type {};

template<typename T, typename = void>
struct has_onValueChange : std::false_type {};
template<typename T>
struct has_onValueChange<T, std::void_t<
    decltype(std::declval<const T&>().onValueChange)
>> : std::true_type {};

} // namespace flux
