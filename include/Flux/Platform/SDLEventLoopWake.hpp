#pragma once

#include <Flux/Platform/EventLoopWake.hpp>

namespace flux {

class SDLEventLoopWake : public EventLoopWake {
public:
    void wake() override;
};

} // namespace flux
