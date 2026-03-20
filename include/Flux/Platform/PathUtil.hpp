#pragma once

#include <string>

namespace flux {

class PathUtil {
public:
    virtual ~PathUtil() = default;
    virtual std::string executableDirectory() = 0;
};

} // namespace flux
