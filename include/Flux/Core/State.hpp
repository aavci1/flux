#pragma once

#include <memory>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <format>

namespace flux {

// Forward declaration
class Application;

// State with automatic frame-based update batching
template<typename T>
class State {
private:
    T value;
    mutable std::shared_mutex mutex;  // Read-write lock for thread safety

    void notifyChange();  // Defined after Application is complete

public:
    State(T initial) : value(std::move(initial)) {}

    // Assignment operators
    State& operator=(const T& newValue) {
        std::unique_lock lock(mutex);
        if constexpr (requires { value == newValue; }) {
            if (value != newValue) {
                value = newValue;
                lock.unlock();  // Release before notifying
                notifyChange();
            }
        } else {
            value = newValue;
            lock.unlock();
            notifyChange();
        }
        return *this;
    }

    State& operator=(T&& newValue) {
        std::unique_lock lock(mutex);
        value = std::move(newValue);
        lock.unlock();
        notifyChange();
        return *this;
    }

    // Arithmetic operators
    template<typename U = T>
    requires requires(U& u) { ++u; }
    State& operator++() {
        std::unique_lock lock(mutex);
        ++value;
        lock.unlock();
        notifyChange();
        return *this;
    }

    template<typename U = T>
    requires requires(U& u) { u++; }
    T operator++(int) {
        std::unique_lock lock(mutex);
        T old = value++;
        lock.unlock();
        notifyChange();
        return old;
    }

    template<typename U = T>
    requires requires(U& u) { --u; }
    State& operator--() {
        std::unique_lock lock(mutex);
        --value;
        lock.unlock();
        notifyChange();
        return *this;
    }

    template<typename U = T>
    requires requires(U& u) { u--; }
    T operator--(int) {
        std::unique_lock lock(mutex);
        T old = value--;
        lock.unlock();
        notifyChange();
        return old;
    }

    template<typename U = T>
    requires requires(U& u, const U& v) { u += v; }
    State& operator+=(const T& val) {
        std::unique_lock lock(mutex);
        value += val;
        lock.unlock();
        notifyChange();
        return *this;
    }

    template<typename U = T>
    requires requires(U& u, const U& v) { u -= v; }
    State& operator-=(const T& val) {
        std::unique_lock lock(mutex);
        value -= val;
        lock.unlock();
        notifyChange();
        return *this;
    }

    // Comparison operators (thread-safe reads)
    template<typename U = T>
    requires requires(const U& u, const U& v) { u == v; }
    bool operator==(const T& other) const {
        std::shared_lock lock(mutex);
        return value == other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u != v; }
    bool operator!=(const T& other) const {
        std::shared_lock lock(mutex);
        return value != other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u < v; }
    bool operator<(const T& other) const {
        std::shared_lock lock(mutex);
        return value < other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u <= v; }
    bool operator<=(const T& other) const {
        std::shared_lock lock(mutex);
        return value <= other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u > v; }
    bool operator>(const T& other) const {
        std::shared_lock lock(mutex);
        return value > other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u >= v; }
    bool operator>=(const T& other) const {
        std::shared_lock lock(mutex);
        return value >= other;
    }

    // Arithmetic with values (thread-safe reads)
    template<typename U = T>
    requires requires(const U& u, const U& v) { u + v; }
    T operator+(const T& other) const {
        std::shared_lock lock(mutex);
        return value + other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u - v; }
    T operator-(const T& other) const {
        std::shared_lock lock(mutex);
        return value - other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u * v; }
    T operator*(const T& other) const {
        std::shared_lock lock(mutex);
        return value * other;
    }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u / v; }
    T operator/(const T& other) const {
        std::shared_lock lock(mutex);
        return value / other;
    }

    // Implicit conversion (thread-safe read)
    operator T() const {
        std::shared_lock lock(mutex);
        return value;
    }

    // Pointer access for containers
    T* operator->() {
        return &value;
    }

    const T* operator->() const {
        return &value;
    }
};

// Deduction guides for Class Template Argument Deduction (CTAD)
// String literals deduce to std::string, not const char*
State(const char*) -> State<std::string>;

// String concatenation operators
inline std::string operator+(const State<std::string>& state, const std::string& str) {
    return static_cast<std::string>(state) + str;
}

inline std::string operator+(const std::string& str, const State<std::string>& state) {
    return str + static_cast<std::string>(state);
}

// Forward declare the actual implementation (defined in Application.cpp or similar)
void requestApplicationRedraw();

// Implementation of notifyChange
template<typename T>
inline void State<T>::notifyChange() {
    requestApplicationRedraw();
}

} // namespace flux

// Format support for State<T>
template<typename T>
struct std::formatter<flux::State<T>> : std::formatter<T> {
    auto format(const flux::State<T>& state, std::format_context& ctx) const {
        return std::formatter<T>::format(static_cast<T>(state), ctx);
    }
};
