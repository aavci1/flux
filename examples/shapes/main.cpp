#include <Flux.hpp>

using namespace flux;

struct Shapes {
    FLUX_VIEW_PROPERTIES;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        ctx.setFillStyle(FillStyle::solid(Colors::red));

        ctx.drawCircle(bounds.center(), 100);
    }
};

int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);

    auto& window = runtime.createWindow({
        .size = {800, 800},
        .title = "Shapes"
    });

    window.setRootView(
        Shapes {}
    );

    return runtime.run();
}