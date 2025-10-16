#pragma once

#include <Flux/Core/State.hpp>
#include <functional>
#include <variant>
#include <memory>
#include <type_traits>

namespace flux {

// Property - A flexible wrapper that accepts values, State<T>, or lambdas
template<typename T>
class Property {
private:
    // Store either: direct value, State reference, or lambda
    std::variant<T, std::reference_wrapper<State<T>>, std::function<T()>> storage;

public:
    // Default constructor
    Property() : storage(T{}) {}

    // Constructor from direct value
    Property(const T& value) : storage(value) {}
    Property(T&& value) : storage(std::move(value)) {}

    // Constructor from initializer list (for containers like std::vector)
    template<typename U>
    Property(std::initializer_list<U> init) : storage(T(init)) {}

    // Constructor from convertible type (e.g., Color to Color)
    template<typename U>
    requires std::is_convertible_v<U, T> && (!std::is_same_v<std::decay_t<U>, T>) &&
             (!std::is_same_v<std::decay_t<U>, State<T>>) && (!std::is_invocable_v<U>)
    Property(U&& value) : storage(T(std::forward<U>(value))) {}

    // Constructor from State (stores reference)
    Property(State<T>& state) : storage(std::ref(state)) {}

    // Constructor from lambda/function
    template<typename F>
    requires std::is_invocable_r_v<T, F> && (!std::is_same_v<std::decay_t<F>, T>) &&
             (!std::is_same_v<std::decay_t<F>, State<T>>)
    Property(F&& func) : storage(std::function<T()>(std::forward<F>(func))) {}

    // Assignment operators
    Property& operator=(const T& value) {
        storage = value;
        return *this;
    }

    Property& operator=(T&& value) {
        storage = std::move(value);
        return *this;
    }

    template<typename U>
    requires std::is_convertible_v<U, T> && (!std::is_same_v<std::decay_t<U>, T>) &&
             (!std::is_same_v<std::decay_t<U>, State<T>>) && (!std::is_invocable_v<U>)
    Property& operator=(U&& value) {
        storage = T(std::forward<U>(value));
        return *this;
    }

    Property& operator=(State<T>& state) {
        storage = std::ref(state);
        return *this;
    }

    template<typename F>
    requires std::is_invocable_r_v<T, F> && (!std::is_same_v<std::decay_t<F>, T>) &&
             (!std::is_same_v<std::decay_t<F>, State<T>>)
    Property& operator=(F&& func) {
        storage = std::function<T()>(std::forward<F>(func));
        return *this;
    }

    // Evaluate the property to get the current value
    T get() const {
        return std::visit([](auto&& arg) -> T {
            using ArgType = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<ArgType, T>) {
                return arg;
            } else if constexpr (std::is_same_v<ArgType, std::reference_wrapper<State<T>>>) {
                return static_cast<T>(arg.get());
            } else if constexpr (std::is_same_v<ArgType, std::function<T()>>) {
                return arg();
            }
        }, storage);
    }

    // Implicit conversion to T
    operator T() const {
        return get();
    }

    // Pointer access for containers (returns temporary - use with caution)
    T operator->() const {
        return get();
    }
};

// Deduction guides for string literals
Property(const char*) -> Property<std::string>;

// Special handling for string concatenation
inline std::string operator+(const Property<std::string>& prop, const std::string& str) {
    return prop.get() + str;
}

inline std::string operator+(const std::string& str, const Property<std::string>& prop) {
    return str + prop.get();
}

// Specialization for std::vector to support initializer lists
template<typename T>
class Property<std::vector<T>> {
private:
    std::variant<std::vector<T>, std::reference_wrapper<State<std::vector<T>>>, std::function<std::vector<T>()>> storage;

public:
    Property() : storage(std::vector<T>{}) {}
    Property(const std::vector<T>& value) : storage(value) {}
    Property(std::vector<T>&& value) : storage(std::move(value)) {}
    Property(std::initializer_list<T> init) : storage(std::vector<T>(init)) {}
    Property(State<std::vector<T>>& state) : storage(std::ref(state)) {}

    template<typename F>
    requires std::is_invocable_r_v<std::vector<T>, F>
    Property(F&& func) : storage(std::function<std::vector<T>()>(std::forward<F>(func))) {}

    Property& operator=(const std::vector<T>& value) {
        storage = value;
        return *this;
    }

    Property& operator=(std::vector<T>&& value) {
        storage = std::move(value);
        return *this;
    }

    Property& operator=(std::initializer_list<T> init) {
        storage = std::vector<T>(init);
        return *this;
    }

    Property& operator=(State<std::vector<T>>& state) {
        storage = std::ref(state);
        return *this;
    }

    template<typename F>
    requires std::is_invocable_r_v<std::vector<T>, F>
    Property& operator=(F&& func) {
        storage = std::function<std::vector<T>()>(std::forward<F>(func));
        return *this;
    }

    std::vector<T> get() const {
        return std::visit([](auto&& arg) -> std::vector<T> {
            using ArgType = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<ArgType, std::vector<T>>) {
                return arg;
            } else if constexpr (std::is_same_v<ArgType, std::reference_wrapper<State<std::vector<T>>>>) {
                return static_cast<std::vector<T>>(arg.get());
            } else if constexpr (std::is_same_v<ArgType, std::function<std::vector<T>()>>) {
                return arg();
            }
        }, storage);
    }

    operator std::vector<T>() const {
        return get();
    }
};

} // namespace flux

// Format support for Property<T>
template<typename T>
struct std::formatter<flux::Property<T>> : std::formatter<T> {
    auto format(const flux::Property<T>& prop, std::format_context& ctx) const {
        return std::formatter<T>::format(prop.get(), ctx);
    }
};

