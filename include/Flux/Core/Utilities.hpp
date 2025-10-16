#pragma once

#include <vector>
#include <functional>

namespace flux {

// Forward declaration
class View;

// Helper function to map over vectors and convert to View (functional style)
template<typename T, typename Func>
auto map(const std::vector<T>& vec, Func&& func) -> std::vector<View> {
    std::vector<View> result;
    result.reserve(vec.size());

    for (const auto& item : vec) {
        result.push_back(func(item));
    }

    return result;
}

} // namespace flux

