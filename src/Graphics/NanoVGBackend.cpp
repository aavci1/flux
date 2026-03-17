#include <Flux/Graphics/NanoVGBackend.hpp>
#include <nanovg.h>
#include <cmath>

namespace flux {

void NanoVGBackend::execute(const RenderCommandBuffer& buffer) {
    for (const auto& cmd : buffer.commands()) {
        std::visit([this](const auto& c) { dispatch(c); }, cmd);
    }
}

void NanoVGBackend::dispatch(const CmdDrawRect& cmd) {
    nvgBeginPath(nvg_);
    if (cmd.cornerRadius.topLeft > 0 || cmd.cornerRadius.topRight > 0 ||
        cmd.cornerRadius.bottomRight > 0 || cmd.cornerRadius.bottomLeft > 0) {
        float avg = (cmd.cornerRadius.topLeft + cmd.cornerRadius.topRight +
                     cmd.cornerRadius.bottomRight + cmd.cornerRadius.bottomLeft) / 4.0f;
        nvgRoundedRect(nvg_, cmd.bounds.x, cmd.bounds.y,
                       cmd.bounds.width, cmd.bounds.height, avg);
    } else {
        nvgRect(nvg_, cmd.bounds.x, cmd.bounds.y, cmd.bounds.width, cmd.bounds.height);
    }

    if (cmd.fillColor.a > 0) {
        nvgFillColor(nvg_, nvgRGBAf(cmd.fillColor.r, cmd.fillColor.g,
                                     cmd.fillColor.b, cmd.fillColor.a));
        nvgFill(nvg_);
    }
    if (cmd.strokeWidth > 0 && cmd.strokeColor.a > 0) {
        nvgStrokeColor(nvg_, nvgRGBAf(cmd.strokeColor.r, cmd.strokeColor.g,
                                       cmd.strokeColor.b, cmd.strokeColor.a));
        nvgStrokeWidth(nvg_, cmd.strokeWidth);
        nvgStroke(nvg_);
    }
}

void NanoVGBackend::dispatch(const CmdDrawCircle& cmd) {
    nvgBeginPath(nvg_);
    nvgCircle(nvg_, cmd.center.x, cmd.center.y, cmd.radius);
    if (cmd.fillColor.a > 0) {
        nvgFillColor(nvg_, nvgRGBAf(cmd.fillColor.r, cmd.fillColor.g,
                                     cmd.fillColor.b, cmd.fillColor.a));
        nvgFill(nvg_);
    }
    if (cmd.strokeWidth > 0 && cmd.strokeColor.a > 0) {
        nvgStrokeColor(nvg_, nvgRGBAf(cmd.strokeColor.r, cmd.strokeColor.g,
                                       cmd.strokeColor.b, cmd.strokeColor.a));
        nvgStrokeWidth(nvg_, cmd.strokeWidth);
        nvgStroke(nvg_);
    }
}

void NanoVGBackend::dispatch(const CmdDrawLine& cmd) {
    nvgBeginPath(nvg_);
    nvgMoveTo(nvg_, cmd.from.x, cmd.from.y);
    nvgLineTo(nvg_, cmd.to.x, cmd.to.y);
    nvgStrokeColor(nvg_, nvgRGBAf(cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a));
    nvgStrokeWidth(nvg_, cmd.width);
    nvgStroke(nvg_);
}

void NanoVGBackend::dispatch(const CmdDrawText& cmd) {
    nvgFontSize(nvg_, cmd.fontSize);
    nvgFillColor(nvg_, nvgRGBAf(cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a));
    if (!cmd.fontFamily.empty()) {
        nvgFontFace(nvg_, cmd.fontFamily.c_str());
    }
    nvgTextAlign(nvg_, cmd.align);
    nvgText(nvg_, cmd.position.x, cmd.position.y, cmd.text.c_str(), nullptr);
}

void NanoVGBackend::dispatch(const CmdDrawImage& cmd) {
    NVGpaint imgPaint = nvgImagePattern(
        nvg_, cmd.destRect.x, cmd.destRect.y,
        cmd.destRect.width, cmd.destRect.height,
        0, cmd.imageHandle, cmd.opacity
    );
    nvgBeginPath(nvg_);
    nvgRect(nvg_, cmd.destRect.x, cmd.destRect.y, cmd.destRect.width, cmd.destRect.height);
    nvgFillPaint(nvg_, imgPaint);
    nvgFill(nvg_);
}

void NanoVGBackend::dispatch(const CmdPushClip& cmd) {
    nvgSave(nvg_);
    nvgScissor(nvg_, cmd.bounds.x, cmd.bounds.y, cmd.bounds.width, cmd.bounds.height);
}

void NanoVGBackend::dispatch(const CmdPopClip&) {
    nvgResetScissor(nvg_);
    nvgRestore(nvg_);
}

void NanoVGBackend::dispatch(const CmdPushTransform& cmd) {
    nvgSave(nvg_);
    nvgTransform(nvg_, cmd.matrix[0], cmd.matrix[1],
                 cmd.matrix[2], cmd.matrix[3],
                 cmd.matrix[4], cmd.matrix[5]);
}

void NanoVGBackend::dispatch(const CmdPopTransform&) {
    nvgRestore(nvg_);
}

void NanoVGBackend::dispatch(const CmdSetOpacity& cmd) {
    nvgGlobalAlpha(nvg_, cmd.opacity);
}

} // namespace flux
