#pragma once

#include <Flux/Core/Theme.hpp>
#include <type_traits>

namespace flux {

struct ThemeKey {
    using Value = Theme;
    static Theme defaultValue() { return Theme::light(); }
};

/**
 * Inherited values for a subtree. Extend with additional `EnvironmentKey` types and `with<Key>()`
 * overloads as needed.
 */
struct Environment {
    Theme theme = ThemeKey::defaultValue();

    static Environment defaults() {
        Environment e;
        e.theme = ThemeKey::defaultValue();
        return e;
    }

    template<typename Key>
    Environment with(typename Key::Value value) const {
        static_assert(std::is_same_v<Key, ThemeKey>, "Add an Environment::with<Key> overload for this key");
        Environment copy = *this;
        copy.theme = std::move(value);
        return copy;
    }
};

inline bool operator==(const Environment& a, const Environment& b) {
    return a.theme == b.theme;
}

} // namespace flux
