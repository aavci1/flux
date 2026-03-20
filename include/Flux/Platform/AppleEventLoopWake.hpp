#pragma once

#include <Flux/Platform/EventLoopWake.hpp>

namespace flux {

class AppleEventLoopWake : public EventLoopWake {
public:
    void wake() override;
};

} // namespace flux
