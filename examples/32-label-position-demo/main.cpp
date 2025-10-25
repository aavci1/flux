#include <Flux.hpp>
#include <iostream>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Property<bool> checkbox1 = false;
    Property<bool> checkbox2 = false;
    Property<bool> toggle1 = false;
    Property<bool> toggle2 = false;
    Property<std::string> radio1 = "left";

    Window window({
        .size = {900, 600},
        .title = "Label Position Demo"
    });


    Property<LabelPosition> alignment = LabelPosition::trailing; 
    Property<JustifyContent> justifyContent = JustifyContent::start;

    window.setRootView(
        HStack {
            .padding = 20,
            .spacing = 20,
            .children = {
                VStack {
                    .borderColor = Colors::gray,
                    .borderWidth = 1,
                    .cornerRadius = 12,
                    .expansionBias = 1.0f,
                    .padding = 20,
                    .spacing = 20,
                    .children = {
                        RadioButton {
                            .selected = [&]() { return alignment == LabelPosition::leading; },
                            .label = "Leading",
                            .onChange = [&]() {
                                alignment = LabelPosition::leading;
                            }
                        },
                        RadioButton {
                            .selected = [&]() { return alignment == LabelPosition::trailing; },
                            .label = "Trailing",
                            .onChange = [&]() {
                                alignment = LabelPosition::trailing;
                            }
                        },
                        RadioButton {
                            .selected = [&]() { return justifyContent == JustifyContent::start; },
                            .label = "Start",
                            .onChange = [&]() {
                                justifyContent = JustifyContent::start;
                            }
                        },
                        RadioButton {
                            .selected = [&]() { return justifyContent == JustifyContent::center; },
                            .label = "Center",
                            .onChange = [&]() {
                                justifyContent = JustifyContent::center;
                            }
                        },
                        RadioButton {
                            .selected = [&]() { return justifyContent == JustifyContent::end; },
                            .label = "End",
                            .onChange = [&]() {
                                justifyContent = JustifyContent::end;
                            }
                        },
                        RadioButton {
                            .selected = [&]() { return justifyContent == JustifyContent::spaceBetween; },
                            .label = "Space Between",
                            .onChange = [&]() {
                                justifyContent = JustifyContent::spaceBetween;
                            }
                        },
                        RadioButton {
                            .selected = [&]() { return justifyContent == JustifyContent::spaceAround; },
                            .label = "Space Around",
                            .onChange = [&]() {
                                justifyContent = JustifyContent::spaceAround;
                            }
                        },
                        RadioButton {
                            .selected = [&]() { return justifyContent == JustifyContent::spaceEvenly; },
                            .label = "Space Evenly",
                            .onChange = [&]() {
                                justifyContent = JustifyContent::spaceEvenly;
                            }
                        }
                    }
                },
                VStack {
                    .borderColor = Colors::gray,
                    .borderWidth = 1,
                    .cornerRadius = 12,
                    .expansionBias = 1.0f,
                    .padding = 20,
                    .spacing = 20,
                    .children = {
                        Checkbox {
                            .label = "Checkbox 1",
                            .labelPosition = alignment,
                            .justifyContent = justifyContent
                        },
                        RadioButton {
                            .label = "Radio 1",
                            .labelPosition = alignment,
                            .justifyContent = justifyContent,
                        },
                        Toggle {
                            .label = "Toggle 1",
                            .labelPosition = alignment,
                            .justifyContent = justifyContent
                        }
                    }
                }
            }
        }
    );

    // window.setRootView(
    //     VStack {
    //         .spacing = 20,
    //         .padding = EdgeInsets(20),
    //         .backgroundColor = Color(0.95f, 0.95f, 0.95f, 1.0f),
    //         .children = {
    //             Text {
    //                 .value = "Label Position Demo",
    //                 .fontSize = 28,
    //                 .fontWeight = FontWeight::bold,
    //                 .color = Colors::darkGray,
    //                 .padding = EdgeInsets(0, 0, 10, 0)
    //             },

    //             // Checkboxes with different label positions
    //             Text {
    //                 .value = "Label on the left",
    //                 .fontSize = 20,
    //                 .fontWeight = FontWeight::semibold,
    //                 .color = Color(0.15f, 0.15f, 0.15f, 1.0f),
    //                 .padding = EdgeInsets(0, 0, 8, 0)
    //             },

    //             VStack {
    //                 .spacing = 10,
    //                 .children = {
    //                     Checkbox {
    //                         .checked = checkbox2,
    //                         .label = "Label on left",
    //                         .labelPosition = LabelPosition::trailing,
    //                         .justifyContent = JustifyContent::spaceBetween,
    //                         .onChange = [&]() {
    //                             std::cout << "Checkbox 2: " << static_cast<bool>(checkbox2) << std::endl;
    //                         }
    //                     },
    //                     RadioButton {
    //                         .selected = [&]() { return radio1.get() == "right"; },
    //                         .label = "Label on left",
    //                         .labelPosition = LabelPosition::leading,
    //                         .justifyContent = JustifyContent::spaceBetween,
    //                         .onChange = [&]() {
    //                             radio1 = "right";
    //                             std::cout << "Selected: right" << std::endl;
    //                         }
    //                     },
    //                     Toggle {
    //                         .isOn = toggle2,
    //                         .label = "Toggle with label on left",
    //                         .labelPosition = LabelPosition::leading,
    //                         .justifyContent = JustifyContent::spaceBetween,
    //                         .onChange = [&]() {
    //                             std::cout << "Toggle 2: " << (static_cast<bool>(toggle2) ? "ON" : "OFF") << std::endl;
    //                         }
    //                     }
    //                 }
    //             },

    //             Divider {},

    //             // Toggles with labels and different positions
    //             Text {
    //                 .value = "Label on the right",
    //                 .fontSize = 20,
    //                 .fontWeight = FontWeight::semibold,
    //                 .color = Color(0.15f, 0.15f, 0.15f, 1.0f),
    //                 .padding = EdgeInsets(0, 0, 8, 0)
    //             },

    //             VStack {
    //                 .spacing = 10,
    //                 .alignItems = AlignItems::start,
    //                 .children = {
    //                     Toggle {
    //                         .isOn = toggle1,
    //                         .label = "Toggle with label on right",
    //                         .labelPosition = LabelPosition::trailing,
    //                         .onChange = [&]() {
    //                             std::cout << "Toggle 1: " << (static_cast<bool>(toggle1) ? "ON" : "OFF") << std::endl;
    //                         }
    //                     },
    //                     Checkbox {
    //                         .checked = checkbox1,
    //                         .label = "Label on right (default)",
    //                         .labelPosition = LabelPosition::trailing,
    //                         .onChange = [&]() {
    //                             std::cout << "Checkbox 1: " << static_cast<bool>(checkbox1) << std::endl;
    //                         }
    //                     },
    //                     RadioButton {
    //                         .selected = [&]() { return radio1.get() == "left"; },
    //                         .label = "Label on right",
    //                         .labelPosition = LabelPosition::trailing,
    //                         .onChange = [&]() {
    //                             radio1 = "left";
    //                             std::cout << "Selected: left" << std::endl;
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // );

    return app.exec();
}

