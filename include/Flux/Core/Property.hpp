#pragma once

#include <functional>
#include <variant>
#include <memory>
#include <type_traits>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <format>

namespace flux {

// Forward declaration
void requestApplicationRedraw();

// Property - A flexible wrapper that can be:
// 1. A stateful reactive value (thread-safe with automatic change notifications)
// 2. A direct value
// 3. A lambda function
template<typename T>
class Property {
private:
    // Propertyful value holder with thread-safety
    struct PropertyfulValue {
        T value;
        mutable std::shared_mutex mutex;
        
        PropertyfulValue(T initial) : value(std::move(initial)) {}
        
        void notifyChange() {
            requestApplicationRedraw();
        }
    };
    
    // Store either: stateful value, direct value, or lambda
    std::variant<
        std::shared_ptr<PropertyfulValue>,  // Propertyful reactive value
        T,                                // Direct value
        std::function<T()>                // Lambda/function
    > storage;
    
    bool isPropertyful() const {
        return std::holds_alternative<std::shared_ptr<PropertyfulValue>>(storage);
    }
    
    std::shared_ptr<PropertyfulValue> getPropertyful() const {
        return std::get<std::shared_ptr<PropertyfulValue>>(storage);
    }

public:
    // Default constructor - creates a stateful value
    Property() : storage(std::make_shared<PropertyfulValue>(T{})) {}

    // Copy constructor - shares the stateful storage
    Property(const Property& other) : storage(other.storage) {}
    
    // Move constructor
    Property(Property&& other) noexcept : storage(std::move(other.storage)) {}

    // Constructor from direct value - creates stateful value
    Property(const T& value) : storage(std::make_shared<PropertyfulValue>(value)) {}
    Property(T&& value) : storage(std::make_shared<PropertyfulValue>(std::move(value))) {}

    // Constructor from initializer list (for containers like std::vector)
    template<typename U>
    Property(std::initializer_list<U> init) : storage(std::make_shared<PropertyfulValue>(T(init))) {}

    // Constructor from convertible type (e.g., Color to Color)
    template<typename U>
    requires std::is_convertible_v<U, T> && 
             (!std::is_same_v<std::decay_t<U>, T>) && 
             (!std::is_same_v<std::decay_t<U>, Property<T>>) &&
             (!std::is_invocable_v<U>)
    Property(U&& value) : storage(std::make_shared<PropertyfulValue>(T(std::forward<U>(value)))) {}

    // Constructor from lambda/function - non-stateful
    template<typename F>
    requires std::is_invocable_r_v<T, F> && (!std::is_same_v<std::decay_t<F>, T>)
    Property(F&& func) : storage(std::function<T()>(std::forward<F>(func))) {}

    // Copy assignment - shares the stateful storage
    Property& operator=(const Property& other) {
        if (this != &other) {
            storage = other.storage;
        }
        return *this;
    }
    
    // Move assignment
    Property& operator=(Property&& other) noexcept {
        if (this != &other) {
            storage = std::move(other.storage);
        }
        return *this;
    }

    // Assignment operators - works on stateful values
    Property& operator=(const T& newValue) {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            if constexpr (requires { stateful->value == newValue; }) {
                if (stateful->value != newValue) {
                    stateful->value = newValue;
                    lock.unlock();
                    stateful->notifyChange();
                }
            } else {
                stateful->value = newValue;
                lock.unlock();
                stateful->notifyChange();
            }
        } else {
            storage = std::make_shared<PropertyfulValue>(newValue);
        }
        return *this;
    }

    Property& operator=(T&& newValue) {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            stateful->value = std::move(newValue);
            lock.unlock();
            stateful->notifyChange();
        } else {
            storage = std::make_shared<PropertyfulValue>(std::move(newValue));
        }
        return *this;
    }

    template<typename U>
    requires std::is_convertible_v<U, T> && 
             (!std::is_same_v<std::decay_t<U>, T>) && 
             (!std::is_same_v<std::decay_t<U>, Property<T>>) &&
             (!std::is_invocable_v<U>)
    Property& operator=(U&& value) {
        return *this = T(std::forward<U>(value));
    }

    template<typename F>
    requires std::is_invocable_r_v<T, F> && (!std::is_same_v<std::decay_t<F>, T>)
    Property& operator=(F&& func) {
        storage = std::function<T()>(std::forward<F>(func));
        return *this;
    }

    // Arithmetic operators (only for stateful values)
    template<typename U = T>
    requires requires(U& u) { ++u; }
    Property& operator++() {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            ++stateful->value;
            lock.unlock();
            stateful->notifyChange();
        }
        return *this;
    }

    template<typename U = T>
    requires requires(U& u) { u++; }
    T operator++(int) {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            T old = stateful->value++;
            lock.unlock();
            stateful->notifyChange();
            return old;
        }
        return T{};
    }

    template<typename U = T>
    requires requires(U& u) { --u; }
    Property& operator--() {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            --stateful->value;
            lock.unlock();
            stateful->notifyChange();
        }
        return *this;
    }

    template<typename U = T>
    requires requires(U& u) { u--; }
    T operator--(int) {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            T old = stateful->value--;
            lock.unlock();
            stateful->notifyChange();
            return old;
        }
        return T{};
    }

    template<typename U = T>
    requires requires(U& u, const U& v) { u += v; }
    Property& operator+=(const T& val) {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            stateful->value += val;
            lock.unlock();
            stateful->notifyChange();
        }
        return *this;
    }

    template<typename U = T>
    requires requires(U& u, const U& v) { u -= v; }
    Property& operator-=(const T& val) {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            stateful->value -= val;
            lock.unlock();
            stateful->notifyChange();
        }
        return *this;
    }

    // Comparison operators (thread-safe reads)
    template<typename U = T>
    requires requires(const U& u, const U& v) { u == v; }
    bool operator==(const T& other) const {
        T val = get();
        return val == other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u != v; }
    bool operator!=(const T& other) const {
        T val = get();
        return val != other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u < v; }
    bool operator<(const T& other) const {
        T val = get();
        return val < other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u <= v; }
    bool operator<=(const T& other) const {
        T val = get();
        return val <= other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u > v; }
    bool operator>(const T& other) const {
        T val = get();
        return val > other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u >= v; }
    bool operator>=(const T& other) const {
        T val = get();
        return val >= other;
    }

    // Arithmetic with values (thread-safe reads)
    template<typename U = T>
    requires requires(const U& u, const U& v) { u + v; }
    T operator+(const T& other) const {
        T val = get();
        return val + other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u - v; }
    T operator-(const T& other) const {
        T val = get();
        return val - other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u * v; }
    T operator*(const T& other) const {
        T val = get();
        return val * other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u / v; }
    T operator/(const T& other) const {
        T val = get();
        return val / other;
    }

    // Evaluate the property to get the current value
    T get() const {
        return std::visit([](auto&& arg) -> T {
            using ArgType = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<ArgType, std::shared_ptr<PropertyfulValue>>) {
                std::shared_lock lock(arg->mutex);
                return arg->value;
            } else if constexpr (std::is_same_v<ArgType, T>) {
                return arg;
            } else if constexpr (std::is_same_v<ArgType, std::function<T()>>) {
                return arg();
            }
        }, storage);
    }

    // Implicit conversion to T
    operator T() const {
        return get();
    }

    // Pointer access for containers
    T operator->() const {
        return get();
    }
};

// Deduction guides for string literals
Property(const char*) -> Property<std::string>;

// Special handling for string concatenation
inline std::string operator+(const Property<std::string>& state, const std::string& str) {
    return state.get() + str;
}

inline std::string operator+(const std::string& str, const Property<std::string>& state) {
    return str + state.get();
}

// Specialization for std::vector to support initializer lists
template<typename T>
class Property<std::vector<T>> {
private:
    struct PropertyfulValue {
        std::vector<T> value;
        mutable std::shared_mutex mutex;
        
        PropertyfulValue(std::vector<T> initial) : value(std::move(initial)) {}
        
        void notifyChange() {
            requestApplicationRedraw();
        }
    };
    
    std::variant<
        std::shared_ptr<PropertyfulValue>,
        std::vector<T>,
        std::function<std::vector<T>()>
    > storage;
    
    bool isPropertyful() const {
        return std::holds_alternative<std::shared_ptr<PropertyfulValue>>(storage);
    }
    
    std::shared_ptr<PropertyfulValue> getPropertyful() const {
        return std::get<std::shared_ptr<PropertyfulValue>>(storage);
    }

public:
    Property() : storage(std::make_shared<PropertyfulValue>(std::vector<T>{})) {}
    
    // Copy constructor - shares the stateful storage
    Property(const Property& other) : storage(other.storage) {}
    
    // Move constructor
    Property(Property&& other) noexcept : storage(std::move(other.storage)) {}
    
    Property(const std::vector<T>& value) : storage(std::make_shared<PropertyfulValue>(value)) {}
    Property(std::vector<T>&& value) : storage(std::make_shared<PropertyfulValue>(std::move(value))) {}
    Property(std::initializer_list<T> init) : storage(std::make_shared<PropertyfulValue>(std::vector<T>(init))) {}

    template<typename F>
    requires std::is_invocable_r_v<std::vector<T>, F>
    Property(F&& func) : storage(std::function<std::vector<T>()>(std::forward<F>(func))) {}

    // Copy assignment - shares the stateful storage
    Property& operator=(const Property& other) {
        if (this != &other) {
            storage = other.storage;
        }
        return *this;
    }
    
    // Move assignment
    Property& operator=(Property&& other) noexcept {
        if (this != &other) {
            storage = std::move(other.storage);
        }
        return *this;
    }

    Property& operator=(const std::vector<T>& value) {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            stateful->value = value;
            lock.unlock();
            stateful->notifyChange();
        } else {
            storage = std::make_shared<PropertyfulValue>(value);
        }
        return *this;
    }

    Property& operator=(std::vector<T>&& value) {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            stateful->value = std::move(value);
            lock.unlock();
            stateful->notifyChange();
        } else {
            storage = std::make_shared<PropertyfulValue>(std::move(value));
        }
        return *this;
    }

    Property& operator=(std::initializer_list<T> init) {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            std::unique_lock lock(stateful->mutex);
            stateful->value = std::vector<T>(init);
            lock.unlock();
            stateful->notifyChange();
        } else {
            storage = std::make_shared<PropertyfulValue>(std::vector<T>(init));
        }
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
            if constexpr (std::is_same_v<ArgType, std::shared_ptr<PropertyfulValue>>) {
                std::shared_lock lock(arg->mutex);
                return arg->value;
            } else if constexpr (std::is_same_v<ArgType, std::vector<T>>) {
                return arg;
            } else if constexpr (std::is_same_v<ArgType, std::function<std::vector<T>()>>) {
                return arg();
            }
        }, storage);
    }

    operator std::vector<T>() const {
        return get();
    }
    
    // Pointer access for containers (returns temporary - use with caution)
    T* operator->() {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            return &stateful->value;
        }
        return nullptr;
    }

    const T* operator->() const {
        if (isPropertyful()) {
            auto stateful = getPropertyful();
            return &stateful->value;
        }
        return nullptr;
    }
};

} // namespace flux

// Format support for Property<T>
template<typename T>
struct std::formatter<flux::Property<T>> : std::formatter<T> {
    auto format(const flux::Property<T>& state, std::format_context& ctx) const {
        return std::formatter<T>::format(state.get(), ctx);
    }
};
