#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    auto sliderVal = Property<float>::shared(0.5f);
    auto steppedVal = Property<float>::shared(50.0f);

    auto& window = app.createWindow({
        .size = {600, 400},
        .title = "Test: Slider"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 16,
            .children = {
                Text {
                    .value = "slider-test-root",
                    .fontSize = 18,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                // Continuous slider (0.0 - 1.0)
                VStack {
                    .spacing = 6,
                    .children = {
                        Text { .value = "section-continuous", .fontSize = 12, .fontWeight = FontWeight::bold, .color = Colors::black },
                        Slider {
                            .value = sliderVal,
                            .minValue = 0.0f,
                            .maxValue = 1.0f,
                            .step = 0.01f,
                            .focusKey = "slider-continuous",
                            .onValueChange = [&](float v) { sliderVal = v; }
                        },
                        Text {
                            .value = [&]() { return std::format("slider-value:{:.2f}", static_cast<float>(sliderVal)); },
                            .fontSize = 12,
                            .color = Colors::black
                        }
                    }
                },

                Divider {},

                // Stepped slider (0 - 100, step=10)
                VStack {
                    .spacing = 6,
                    .children = {
                        Text { .value = "section-stepped", .fontSize = 12, .fontWeight = FontWeight::bold, .color = Colors::black },
                        Slider {
                            .value = steppedVal,
                            .minValue = 0.0f,
                            .maxValue = 100.0f,
                            .step = 10.0f,
                            .focusKey = "slider-stepped",
                            .onValueChange = [&](float v) { steppedVal = v; }
                        },
                        Text {
                            .value = [&]() { return std::format("stepped-value:{:.0f}", static_cast<float>(steppedVal)); },
                            .fontSize = 12,
                            .color = Colors::black
                        }
                    }
                }
            }
        }
    );

    return app.exec();
}
