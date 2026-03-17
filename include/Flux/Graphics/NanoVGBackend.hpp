#pragma once

#include <Flux/Graphics/RenderCommandBuffer.hpp>

struct NVGcontext;

namespace flux {

class NanoVGBackend : public RenderBackend {
public:
    explicit NanoVGBackend(NVGcontext* ctx) : nvg_(ctx) {}

    void execute(const RenderCommandBuffer& buffer) override;

private:
    NVGcontext* nvg_;

    void dispatch(const CmdDrawRect& cmd);
    void dispatch(const CmdDrawCircle& cmd);
    void dispatch(const CmdDrawLine& cmd);
    void dispatch(const CmdDrawText& cmd);
    void dispatch(const CmdDrawImage& cmd);
    void dispatch(const CmdPushClip& cmd);
    void dispatch(const CmdPopClip& cmd);
    void dispatch(const CmdPushTransform& cmd);
    void dispatch(const CmdPopTransform& cmd);
    void dispatch(const CmdSetOpacity& cmd);
};

} // namespace flux
