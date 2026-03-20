#pragma once

#include <Flux/Platform/PathUtil.hpp>

namespace flux {

class LinuxPathUtil : public PathUtil {
public:
    std::string executableDirectory() override;
};

} // namespace flux
