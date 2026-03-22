#pragma once

#include <Flux/Core/Types.hpp>

namespace flux {

/**
 * Semantic UI colors. `Theme::dark()` follows VS Code / Cursor **Dark Modern** token names
 * (see extensions/theme-defaults/themes/dark_modern.json).
 */
struct Theme {
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
    float inputBorderWidth{};    // input.borderWidth
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
    Color focusRing{};           // Focus ring outline
    Color selection{};           // Text selection highlight
    Color controlBackground{};   // Dropdown/select trigger surface
    Color shadow{};              // Drop shadows

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
    t.inputBackground = Color::hex(0xFFFFFF);
    t.inputForeground = t.foreground;
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
    t.selection = Color(0.0f, 0.47f, 0.83f, 0.3f);
    t.controlBackground = Color::hex(0xF0F0F0);
    t.shadow = Color(0, 0, 0, 0.15f);
    return t;
}

inline Theme Theme::dark() {
    Theme t;
    // Core surfaces (Dark Modern)
    t.background = Color::hex(0x1F1F1F);       // editor.background
    t.surface = Color::hex(0x181818);         // sideBar, panel, activityBar
    t.surfaceElevated = Color::hex(0x202020); // editorWidget.background
    t.foreground = Color::hex(0xCCCCCC);      // editor.foreground, foreground
    t.secondaryForeground = Color::hex(0x9D9D9D); // descriptionForeground
    t.tertiaryForeground = Color::hex(0x6E7681);  // editorLineNumber.foreground
    t.border = Color::hex(0x2B2B2B);          // sideBar.border, panel.border
    t.borderStrong = Color::hex(0x3C3C3C);    // input.border, dropdown.border
    t.accent = Color::hex(0x0078D4);          // focusBorder, button.background
    t.accentHover = Color::hex(0x026EC1);     // button.hoverBackground
    t.onAccent = Color::hex(0xFFFFFF);        // button.foreground on primary
    t.error = Color::hex(0xF85149);           // errorForeground
    t.inputBackground = Color::hex(0x313131);  // input.background
    t.inputForeground = Color::hex(0xCCCCCC); // input.foreground
    t.inputBorderColor = Color::hex(0x4A4A4A);     // input.borderColor
    t.inputBorderWidth = 1.0f;
    t.inputCornerRadius = CornerRadius(4.0f);
    t.placeholder = Color::hex(0x989898);     // input.placeholderForeground
    t.link = Color::hex(0x4daafc);            // textLink.foreground
    t.codeBackground = Color::hex(0x2B2B2B);  // textCodeBlock.background
    t.badgeBackground = Color::hex(0x616161); // badge.background
    t.badgeForeground = Color::hex(0xF8F8F8);  // badge.foreground
    t.overlay = Color(0, 0, 0, 0.6f);
    t.trackInactive = Color::hex(0x4A4A4A);
    t.success = Color::hex(0x4CAF50);
    t.destructive = Color::hex(0xF44336);
    t.focusRing = t.accent;
    t.selection = Color(0.0f, 0.47f, 0.83f, 0.3f);
    t.controlBackground = Color::hex(0x2D2D2D);
    t.shadow = Color(0, 0, 0, 0.35f);
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
           a.placeholder == b.placeholder && a.link == b.link &&
           a.codeBackground == b.codeBackground && a.badgeBackground == b.badgeBackground &&
           a.badgeForeground == b.badgeForeground && a.overlay == b.overlay &&
           a.trackInactive == b.trackInactive && a.success == b.success &&
           a.destructive == b.destructive && a.focusRing == b.focusRing &&
           a.selection == b.selection && a.controlBackground == b.controlBackground &&
           a.shadow == b.shadow;
}

} // namespace flux
