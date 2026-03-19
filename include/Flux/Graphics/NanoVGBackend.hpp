#pragma once

#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <unordered_map>
#include <string>

struct NVGcontext;

namespace flux {

class NanoVGBackend : public RenderBackend {
public:
    explicit NanoVGBackend(NVGcontext* ctx) : nvg_(ctx) {}

    void execute(const RenderCommandBuffer& buffer) override;

    std::unordered_map<std::string, int>& fontCache() { return fontCache_; }
    std::unordered_map<std::string, int>& imageCache() { return imageCache_; }

private:
    NVGcontext* nvg_;
    std::unordered_map<std::string, int> fontCache_;
    std::unordered_map<std::string, int> imageCache_;

    FillStyle currentFill_;
    StrokeStyle currentStroke_;
    TextStyle currentText_;

    void dispatch(const CmdSave&);
    void dispatch(const CmdRestore&);
    void dispatch(const CmdTranslate&);
    void dispatch(const CmdRotate&);
    void dispatch(const CmdScale&);
    void dispatch(const CmdSetOpacity&);
    void dispatch(const CmdSetFillStyle&);
    void dispatch(const CmdSetStrokeStyle&);
    void dispatch(const CmdSetTextStyle&);
    void dispatch(const CmdDrawRect&);
    void dispatch(const CmdDrawCircle&);
    void dispatch(const CmdDrawLine&);
    void dispatch(const CmdDrawPath&);
    void dispatch(const CmdDrawText&);
    void dispatch(const CmdDrawTextBox&);
    void dispatch(const CmdDrawImage&);
    void dispatch(const CmdDrawImagePath&);
    void dispatch(const CmdClipPath&);
    void dispatch(const CmdClear&);

    void applyFill();
    void applyStroke();
    int resolveFont(const std::string& name, FontWeight weight);
};

} // namespace flux
