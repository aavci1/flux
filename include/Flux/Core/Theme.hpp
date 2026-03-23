#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/Typography.hpp>
#include <string>

namespace flux {

/**
 * Semantic UI colors and layout tokens. `Theme::dark()` follows VS Code / Cursor **Dark Modern** token names
 * (see extensions/theme-defaults/themes/dark_modern.json). Typography sizes align with `Typography` defaults
 * unless overridden here for app-wide consistency.
 */
struct Theme {
    // --- Colors (semantic) -------------------------------------------------
    Color background{};          // editor.background
    Color surface{};             // sideBar.background, panel.background, activityBar.background
    Color surfaceElevated{};     // editorWidget.background
    Color foreground{};          // editor.foreground
    Color secondaryForeground{}; // descriptionForeground
    Color tertiaryForeground{};  // editorLineNumber.foreground (muted)
    Color border{};              // sideBar.border, panel.border
    Color borderStrong{};        // input.border, dropdown.border
    Color accent{};              // focusBorder, button.background
    Color accentHover{};         // button.hoverBackground
    Color onAccent{};            // text on accent (button.foreground)
    Color error{};               // errorForeground
    Color inputBackground{};     // input.background
    Color inputForeground{};     // input.foreground
    Color inputBorderColor{};    // input.borderColor
    float inputBorderWidth{};    // input.borderWidth (0 = no input chrome stroke / no focus ring on inputs)
    CornerRadius inputCornerRadius{};   // input.borderRadius
    Color placeholder{};         // input.placeholderForeground
    Color link{};                // textLink.foreground
    Color codeBackground{};      // textCodeBlock.background
    Color badgeBackground{};     // badge.background
    Color badgeForeground{};     // badge.foreground
    Color overlay{};             // Modal/dialog backdrop
    Color trackInactive{};       // Slider/ProgressBar inactive track
    Color success{};             // Toggle-on, success states
    Color destructive{};         // Destructive actions (delete buttons)
    Color focusRing{};           // Single focus accent (inputs, buttons, controls)
    float focusRingWidth{};      // Stroke width for focus rings where applicable
    Color selection{};           // Text selection highlight
    Color controlBackground{};   // Dropdown/select trigger surface
    Color shadow{};              // Drop shadows

    // --- Typography & spacing (application-wide) ---------------------------
    std::string uiFontFamily = "default";
    float fontSizeBody = Typography::body;
    float fontSizeCallout = Typography::callout;
    float fontSizeCaption = Typography::caption;
    float spacingUnit = 4.0f;
    float inputPadding = 8.0f;           // Default horizontal/vertical padding inside text fields

    // --- Primary button defaults (override per `Button` via view properties) ---
    Color buttonBackground{};              // Filled button surface (defaults to accent)
    Color buttonForeground{};              // Label on filled button (defaults to onAccent)
    Color buttonBorderColor{};             // Outline button stroke
    float buttonBorderWidth = 0.0f;        // 0 = borderless default for chrome; focus ring follows border
    CornerRadius buttonCornerRadius{};
    EdgeInsets buttonPadding = EdgeInsets(12.0f, 24.0f);
    float buttonFontSize = Typography::callout;

    static Theme light();
    /** Cursor / VS Code Dark Modern palette */
    static Theme dark();
};

inline Theme Theme::light() {
    Theme t;
    t.background = Colors::white;
    t.surface = Color::hex(0xF3F3F3);
    t.surfaceElevated = Color::hex(0xFFFFFF);
    t.foreground = Color::hex(0x1A1A1A);
    t.secondaryForeground = Color::hex(0x616161);
    t.tertiaryForeground = Color::hex(0x8A8A8A);
    t.border = Color::hex(0xE5E5E5);
    t.borderStrong = Color::hex(0xCECECE);
    t.accent = Color::hex(0x0078D4);
    t.accentHover = Color::hex(0x026EC1);
    t.onAccent = Color::hex(0xFFFFFF);
    t.error = Color::hex(0xF85149);
    t.inputBackground = Color::hex(0xF5F5F5);
    t.inputForeground = t.foreground;
    t.inputBorderColor = t.borderStrong;
    t.inputBorderWidth = 1.0f;
    t.inputCornerRadius = CornerRadius(4.0f);
    t.placeholder = Color::hex(0x767676);
    t.link = Color::hex(0x006AB1);
    t.codeBackground = Color::hex(0xF5F5F5);
    t.badgeBackground = Color::hex(0x616161);
    t.badgeForeground = Color::hex(0xF8F8F8);
    t.overlay = Color(0, 0, 0, 0.4f);
    t.trackInactive = Color::hex(0xD4D4D4);
    t.success = Color::hex(0x4CAF50);
    t.destructive = Color::hex(0xF44336);
    t.focusRing = t.accent;
    t.focusRingWidth = 2.0f;
    t.selection = Color(0.0f, 0.47f, 0.83f, 0.3f);
    t.controlBackground = Color::hex(0xF0F0F0);
    t.shadow = Color(0, 0, 0, 0.15f);

    t.uiFontFamily = "default";
    t.inputPadding = 8.0f;
    t.buttonBackground = t.accent;
    t.buttonForeground = t.onAccent;
    t.buttonBorderColor = t.borderStrong;
    t.buttonBorderWidth = 0.0f;
    t.buttonCornerRadius = CornerRadius(6.0f);
    t.buttonPadding = EdgeInsets(12.0f, 24.0f);
    t.buttonFontSize = Typography::callout;
    return t;
}

inline Theme Theme::dark() {
    Theme t;
    t.background = Color::hex(0x1F1F1F);
    t.surface = Color::hex(0x181818);
    t.surfaceElevated = Color::hex(0x202020);
    t.foreground = Color::hex(0xCCCCCC);
    t.secondaryForeground = Color::hex(0x9D9D9D);
    t.tertiaryForeground = Color::hex(0x6E7681);
    t.border = Color::hex(0x2B2B2B);
    t.borderStrong = Color::hex(0x3C3C3C);
    t.accent = Color::hex(0x0078D4);
    t.accentHover = Color::hex(0x026EC1);
    t.onAccent = Color::hex(0xFFFFFF);
    t.error = Color::hex(0xF85149);
    t.inputBackground = Color::hex(0x313131);
    t.inputForeground = Color::hex(0xCCCCCC);
    t.inputBorderColor = Color::hex(0x4A4A4A);
    t.inputBorderWidth = 1.0f;
    t.inputCornerRadius = CornerRadius(4.0f);
    t.placeholder = Color::hex(0x989898);
    t.link = Color::hex(0x4daafc);
    t.codeBackground = Color::hex(0x2B2B2B);
    t.badgeBackground = Color::hex(0x616161);
    t.badgeForeground = Color::hex(0xF8F8F8);
    t.overlay = Color(0, 0, 0, 0.6f);
    t.trackInactive = Color::hex(0x4A4A4A);
    t.success = Color::hex(0x4CAF50);
    t.destructive = Color::hex(0xF44336);
    t.focusRing = t.accent;
    t.focusRingWidth = 2.0f;
    t.selection = Color(0.0f, 0.47f, 0.83f, 0.3f);
    t.controlBackground = Color::hex(0x2D2D2D);
    t.shadow = Color(0, 0, 0, 0.35f);

    t.uiFontFamily = "default";
    t.inputPadding = 8.0f;
    t.buttonBackground = t.accent;
    t.buttonForeground = t.onAccent;
    t.buttonBorderColor = t.borderStrong;
    t.buttonBorderWidth = 0.0f;
    t.buttonCornerRadius = CornerRadius(6.0f);
    t.buttonPadding = EdgeInsets(12.0f, 24.0f);
    t.buttonFontSize = Typography::callout;
    return t;
}

inline bool operator==(const Theme& a, const Theme& b) {
    return a.background == b.background && a.surface == b.surface &&
           a.surfaceElevated == b.surfaceElevated && a.foreground == b.foreground &&
           a.secondaryForeground == b.secondaryForeground &&
           a.tertiaryForeground == b.tertiaryForeground && a.border == b.border &&
           a.borderStrong == b.borderStrong && a.accent == b.accent &&
           a.accentHover == b.accentHover && a.onAccent == b.onAccent && a.error == b.error &&
           a.inputBackground == b.inputBackground && a.inputForeground == b.inputForeground &&
           a.inputBorderColor == b.inputBorderColor && a.inputBorderWidth == b.inputBorderWidth &&
           a.inputCornerRadius == b.inputCornerRadius &&
           a.placeholder == b.placeholder && a.link == b.link &&
           a.codeBackground == b.codeBackground && a.badgeBackground == b.badgeBackground &&
           a.badgeForeground == b.badgeForeground && a.overlay == b.overlay &&
           a.trackInactive == b.trackInactive && a.success == b.success &&
           a.destructive == b.destructive && a.focusRing == b.focusRing &&
           a.focusRingWidth == b.focusRingWidth &&
           a.selection == b.selection && a.controlBackground == b.controlBackground &&
           a.shadow == b.shadow && a.uiFontFamily == b.uiFontFamily &&
           a.fontSizeBody == b.fontSizeBody && a.fontSizeCallout == b.fontSizeCallout &&
           a.fontSizeCaption == b.fontSizeCaption && a.spacingUnit == b.spacingUnit &&
           a.inputPadding == b.inputPadding &&
           a.buttonBackground == b.buttonBackground && a.buttonForeground == b.buttonForeground &&
           a.buttonBorderColor == b.buttonBorderColor && a.buttonBorderWidth == b.buttonBorderWidth &&
           a.buttonCornerRadius == b.buttonCornerRadius && a.buttonPadding == b.buttonPadding &&
           a.buttonFontSize == b.buttonFontSize;
}

} // namespace flux
