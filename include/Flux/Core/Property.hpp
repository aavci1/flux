#pragma once

#include <functional>
#include <variant>
#include <memory>
#include <type_traits>
#include <concepts>
#include <atomic>
#include <string>
#include <format>

namespace flux {

class Element;
void requestApplicationRedraw();
void requestRedrawOnly();
void suppressRedrawRequests();
void resumeRedrawRequests();
uint64_t currentBodyGeneration();

// Property<T> — a flexible wrapper with three storage modes:
//
//   Inline (default)  — stores T directly, zero heap allocation, no mutex.
//                        Suitable for view properties set via designated initializers.
//                        Copies are independent (value semantics).
//
//   Shared (opt-in)   — heap-allocated via shared_ptr, supports binding (copies
//                        share storage) and owner-targeted dirty notification.
//                        Create with Property<T>::shared(value).
//
//   Computed (lambda)  — evaluates a std::function<T()> on every read.
//
template<typename T>
class Property {
private:
    struct SharedState {
        T value;
        Element* owner = nullptr;

        SharedState(T initial) : value(std::move(initial)) {}

        void notifyChange();
    };

    // Inline value at index 0 — hot path for the common case (zero-cost reads).
    std::variant<
        T,                              // Inline value (default)
        std::shared_ptr<SharedState>,   // Shared reactive state (opt-in)
        std::function<T()>              // Computed / lambda
    > storage_;

    bool isShared() const {
        return std::holds_alternative<std::shared_ptr<SharedState>>(storage_);
    }

    std::shared_ptr<SharedState> getShared() const {
        return std::get<std::shared_ptr<SharedState>>(storage_);
    }

public:
    Property() : storage_(T{}) {}

    Property(const Property& other) : storage_(other.storage_) {}
    Property(Property&& other) noexcept : storage_(std::move(other.storage_)) {}

    Property(const T& value) : storage_(value) {}
    Property(T&& value) : storage_(std::move(value)) {}

    template<typename U>
    Property(std::initializer_list<U> init) : storage_(T(init)) {}

    template<typename U>
    requires std::is_convertible_v<U, T> &&
             (!std::is_same_v<std::decay_t<U>, T>) &&
             (!std::is_same_v<std::decay_t<U>, Property<T>>) &&
             (!std::is_invocable_v<U>)
    Property(U&& value) : storage_(T(std::forward<U>(value))) {}

    template<typename F>
    requires std::is_invocable_r_v<T, F> && (!std::is_same_v<std::decay_t<F>, T>)
    Property(F&& func) : storage_(std::function<T()>(std::forward<F>(func))) {}

    // Shared reactive factory — heap-allocated, supports binding (copies share
    // storage) and owner-targeted dirty notification.
    static Property shared(T value) {
        Property p;
        p.storage_ = std::make_shared<SharedState>(std::move(value));
        return p;
    }

    Property& operator=(const Property& other) {
        if (this != &other) {
            storage_ = other.storage_;
        }
        return *this;
    }

    Property& operator=(Property&& other) noexcept {
        if (this != &other) {
            storage_ = std::move(other.storage_);
        }
        return *this;
    }

    Property& operator=(const T& newValue) {
        if (isShared()) {
            auto ss = getShared();
            if constexpr (requires { ss->value == newValue; }) {
                if (ss->value == newValue) return *this;
            }
            ss->value = newValue;
            ss->notifyChange();
        } else if (std::holds_alternative<T>(storage_)) {
            if constexpr (requires(const T& a, const T& b) { a == b; }) {
                if (std::get<T>(storage_) == newValue) return *this;
            }
            std::get<T>(storage_) = newValue;
            requestApplicationRedraw();
        } else {
            storage_ = newValue;
            requestApplicationRedraw();
        }
        return *this;
    }

    Property& operator=(T&& newValue) {
        if (isShared()) {
            auto ss = getShared();
            if constexpr (requires { ss->value == newValue; }) {
                if (ss->value == newValue) return *this;
            }
            ss->value = std::move(newValue);
            ss->notifyChange();
        } else if (std::holds_alternative<T>(storage_)) {
            if constexpr (requires(const T& a, const T& b) { a == b; }) {
                if (std::get<T>(storage_) == newValue) return *this;
            }
            std::get<T>(storage_) = std::move(newValue);
            requestApplicationRedraw();
        } else {
            storage_ = std::move(newValue);
            requestApplicationRedraw();
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
        storage_ = std::function<T()>(std::forward<F>(func));
        return *this;
    }

    // Arithmetic operators
    template<typename U = T>
    requires requires(U& u) { ++u; }
    Property& operator++() {
        if (isShared()) {
            auto ss = getShared();
            ++ss->value;
            ss->notifyChange();
        } else if (std::holds_alternative<T>(storage_)) {
            ++std::get<T>(storage_);
            requestApplicationRedraw();
        }
        return *this;
    }

    template<typename U = T>
    requires requires(U& u) { u++; }
    T operator++(int) {
        if (isShared()) {
            auto ss = getShared();
            T old = ss->value++;
            ss->notifyChange();
            return old;
        } else if (std::holds_alternative<T>(storage_)) {
            T old = std::get<T>(storage_)++;
            requestApplicationRedraw();
            return old;
        }
        return T{};
    }

    template<typename U = T>
    requires requires(U& u) { --u; }
    Property& operator--() {
        if (isShared()) {
            auto ss = getShared();
            --ss->value;
            ss->notifyChange();
        } else if (std::holds_alternative<T>(storage_)) {
            --std::get<T>(storage_);
            requestApplicationRedraw();
        }
        return *this;
    }

    template<typename U = T>
    requires requires(U& u) { u--; }
    T operator--(int) {
        if (isShared()) {
            auto ss = getShared();
            T old = ss->value--;
            ss->notifyChange();
            return old;
        } else if (std::holds_alternative<T>(storage_)) {
            T old = std::get<T>(storage_)--;
            requestApplicationRedraw();
            return old;
        }
        return T{};
    }

    template<typename U = T>
    requires requires(U& u, const U& v) { u += v; }
    Property& operator+=(const T& val) {
        if (isShared()) {
            auto ss = getShared();
            ss->value += val;
            ss->notifyChange();
        } else if (std::holds_alternative<T>(storage_)) {
            std::get<T>(storage_) += val;
            requestApplicationRedraw();
        }
        return *this;
    }

    template<typename U = T>
    requires requires(U& u, const U& v) { u -= v; }
    Property& operator-=(const T& val) {
        if (isShared()) {
            auto ss = getShared();
            ss->value -= val;
            ss->notifyChange();
        } else if (std::holds_alternative<T>(storage_)) {
            std::get<T>(storage_) -= val;
            requestApplicationRedraw();
        }
        return *this;
    }

    // Comparison operators
    template<typename U = T>
    requires requires(const U& u, const U& v) { u == v; }
    bool operator==(const T& other) const { return get() == other; }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u != v; }
    bool operator!=(const T& other) const { return get() != other; }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u < v; }
    bool operator<(const T& other) const { return get() < other; }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u <= v; }
    bool operator<=(const T& other) const { return get() <= other; }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u > v; }
    bool operator>(const T& other) const { return get() > other; }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u >= v; }
    bool operator>=(const T& other) const { return get() >= other; }

    // Arithmetic with values
    template<typename U = T>
    requires requires(const U& u, const U& v) { u + v; }
    T operator+(const T& other) const { return get() + other; }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u - v; }
    T operator-(const T& other) const { return get() - other; }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u * v; }
    T operator*(const T& other) const { return get() * other; }

    template<typename U = T>
    requires requires(const U& u, const U& v) { u / v; }
    T operator/(const T& other) const { return get() / other; }

    // Read — fast path for inline (no mutex, no visit overhead)
    T get() const {
        if (auto* val = std::get_if<T>(&storage_)) {
            return *val;
        }
        if (auto* ss = std::get_if<std::shared_ptr<SharedState>>(&storage_)) {
            return (*ss)->value;
        }
        return std::get<std::function<T()>>(storage_)();
    }

    operator T() const { return get(); }

    T operator->() const { return get(); }

    void setOwner(Element* elem) {
        if (isShared()) {
            getShared()->owner = elem;
        }
    }
};

Property(const char*) -> Property<std::string>;

inline std::string operator+(const Property<std::string>& state, const std::string& str) {
    return state.get() + str;
}

inline std::string operator+(const std::string& str, const Property<std::string>& state) {
    return str + state.get();
}

// Specialization for std::vector<T> — same tiered storage model
template<typename T>
class Property<std::vector<T>> {
private:
    struct SharedState {
        std::vector<T> value;
        Element* owner = nullptr;

        SharedState(std::vector<T> initial) : value(std::move(initial)) {}

        void notifyChange();
    };

    std::variant<
        std::vector<T>,
        std::shared_ptr<SharedState>,
        std::function<std::vector<T>()>
    > storage_;

    bool isShared() const {
        return std::holds_alternative<std::shared_ptr<SharedState>>(storage_);
    }

    std::shared_ptr<SharedState> getShared() const {
        return std::get<std::shared_ptr<SharedState>>(storage_);
    }

public:
    Property() : storage_(std::vector<T>{}) {}

    Property(const Property& other) : storage_(other.storage_) {}
    Property(Property&& other) noexcept : storage_(std::move(other.storage_)) {}

    Property(const std::vector<T>& value) : storage_(value) {}
    Property(std::vector<T>&& value) : storage_(std::move(value)) {}
    Property(std::initializer_list<T> init) : storage_(std::vector<T>(init)) {}

    template<typename F>
    requires std::is_invocable_r_v<std::vector<T>, F>
    Property(F&& func) : storage_(std::function<std::vector<T>()>(std::forward<F>(func))) {}

    static Property shared(std::vector<T> value) {
        Property p;
        p.storage_ = std::make_shared<SharedState>(std::move(value));
        return p;
    }

    Property& operator=(const Property& other) {
        if (this != &other) storage_ = other.storage_;
        return *this;
    }

    Property& operator=(Property&& other) noexcept {
        if (this != &other) storage_ = std::move(other.storage_);
        return *this;
    }

    Property& operator=(const std::vector<T>& value) {
        if (isShared()) {
            auto ss = getShared();
            if constexpr (std::equality_comparable<T>) {
                if (ss->value == value) return *this;
            }
            ss->value = value;
            ss->notifyChange();
        } else {
            storage_ = value;
            requestApplicationRedraw();
        }
        return *this;
    }

    Property& operator=(std::vector<T>&& value) {
        if (isShared()) {
            auto ss = getShared();
            if constexpr (std::equality_comparable<T>) {
                if (ss->value == value) return *this;
            }
            ss->value = std::move(value);
            ss->notifyChange();
        } else {
            storage_ = std::move(value);
            requestApplicationRedraw();
        }
        return *this;
    }

    Property& operator=(std::initializer_list<T> init) {
        if (isShared()) {
            auto ss = getShared();
            ss->value = std::vector<T>(init);
            ss->notifyChange();
        } else {
            storage_ = std::vector<T>(init);
            requestApplicationRedraw();
        }
        return *this;
    }

    template<typename F>
    requires std::is_invocable_r_v<std::vector<T>, F>
    Property& operator=(F&& func) {
        storage_ = std::function<std::vector<T>()>(std::forward<F>(func));
        return *this;
    }

    std::vector<T> get() const {
        if (auto* val = std::get_if<std::vector<T>>(&storage_)) {
            return *val;
        }
        if (auto* ss = std::get_if<std::shared_ptr<SharedState>>(&storage_)) {
            return (*ss)->value;
        }
        return std::get<std::function<std::vector<T>()>>(storage_)();
    }

    operator std::vector<T>() const { return get(); }

    T* operator->() {
        if (isShared()) return &getShared()->value;
        if (auto* val = std::get_if<std::vector<T>>(&storage_)) return val->data();
        return nullptr;
    }

    const T* operator->() const {
        if (isShared()) return &getShared()->value;
        if (auto* val = std::get_if<std::vector<T>>(&storage_)) return val->data();
        return nullptr;
    }

    void setOwner(Element* elem) {
        if (isShared()) getShared()->owner = elem;
    }
};

} // namespace flux

#include <Flux/Core/Element.hpp>

namespace flux {

template<typename T>
void Property<T>::SharedState::notifyChange() {
    if (owner) {
        owner->markDirty();
    } else {
        requestApplicationRedraw();
    }
}

template<typename T>
void Property<std::vector<T>>::SharedState::notifyChange() {
    if (owner) {
        owner->markDirty();
    } else {
        requestApplicationRedraw();
    }
}

} // namespace flux

template<typename T>
struct std::formatter<flux::Property<T>> : std::formatter<T> {
    auto format(const flux::Property<T>& state, std::format_context& ctx) const {
        return std::formatter<T>::format(state.get(), ctx);
    }
};
