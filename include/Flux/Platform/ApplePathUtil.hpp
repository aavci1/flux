#pragma once

#include <Flux/Platform/PathUtil.hpp>

namespace flux {

class ApplePathUtil : public PathUtil {
public:
    std::string executableDirectory() override;
};

} // namespace flux
