#include <Flux.hpp>
#include <iostream>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // State management
    Property<bool> checkbox1 = false;
    Property<bool> checkbox2 = true;
    Property<bool> checkbox3 = false;

    Property<bool> toggle1 = false;
    Property<bool> toggle2 = true;

    Property<std::string> selectedOption = "option1";

    Property<float> progress1 = 0.0f;
    Property<float> progress2 = 0.65f;

    Property<float> sliderValue1 = 0.5f;
    Property<float> sliderValue2 = 75.0f;
    Property<float> sliderValue3 = 0.3f;

    Property<int> clickCount = 0;

    // Auto-increment progress for demo
    Property<bool> animateProgress = true;

    Window window({
        .size = {1400, 900},
        .title = "Flux Component Showcase"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 20,
            .backgroundColor = Color(0.95f, 0.95f, 0.95f, 1.0f),
            .children = {
                // Title
                Text {
                    .value = "Flux Component Showcase",
                    .fontSize = 32,
                    .fontWeight = FontWeight::bold,
                    .color = Color(0.2f, 0.2f, 0.2f, 1.0f),
                    .padding = EdgeInsets(0, 0, 10, 0)
                },

                Divider {
                    .thickness = 2.0f,
                    .color = Colors::gray
                },

                // Main content with two columns
                HStack {
                    .spacing = 20,
                    .alignItems = AlignItems::start,
                    .expansionBias = 1.0f,
                    .children = {
                        // Left column
                        VStack {
                            .spacing = 20,
                            .expansionBias = 1.0f,
                            .backgroundColor = Colors::white,
                            .padding = 20,
                            .cornerRadius = 8,
                            .borderColor = Color(0.85f, 0.85f, 0.85f, 1.0f),
                            .borderWidth = 1,
                            .children = {
                                // Checkboxes Section
                                Text {
                                    .value = "Checkboxes",
                                    .fontSize = 22,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color(0.15f, 0.15f, 0.15f, 1.0f),
                                    .padding = EdgeInsets(0, 0, 8, 0)
                                },

                                VStack {
                                    .spacing = 10,
                                    .alignItems = AlignItems::start,
                                    .children = {
                                        Checkbox {
                                            .checked = checkbox1,
                                            .label = "Enable notifications",
                                            .onChange = [&]() {
                                                std::cout << "Checkbox 1: " << static_cast<bool>(checkbox1) << std::endl;
                                            }
                                        },
                                        Checkbox {
                                            .checked = checkbox2,
                                            .label = "Auto-save changes",
                                            .onChange = [&]() {
                                                std::cout << "Checkbox 2: " << static_cast<bool>(checkbox2) << std::endl;
                                            }
                                        },
                                        Checkbox {
                                            .checked = checkbox3,
                                            .label = "Dark mode"
                                        }
                                    }
                                },

                                Divider {},

                                // Toggles Section
                                Text {
                                    .value = "Toggle Switches",
                                    .fontSize = 22,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color(0.15f, 0.15f, 0.15f, 1.0f),
                                    .padding = EdgeInsets(0, 0, 8, 0)
                                },

                                HStack {
                                    .spacing = 20,
                                    .alignItems = AlignItems::center,
                                    .children = {
                                        Toggle {
                                            .isOn = toggle1,
                                            .onChange = [&]() {
                                                std::cout << "Toggle 1: " << (static_cast<bool>(toggle1) ? "ON" : "OFF") << std::endl;
                                            }
                                        },
                                        Text {
                                            .value = [&]() {
                                                return static_cast<bool>(toggle1) ? "Enabled" : "Disabled";
                                            },
                                            .fontSize = 14
                                        }
                                    }
                                },

                                HStack {
                                    .spacing = 20,
                                    .alignItems = AlignItems::center,
                                    .children = {
                                        Toggle {
                                            .isOn = toggle2,
                                            .onColor = Color::hex(0xFF5722)
                                        },
                                        Text {
                                            .value = [&]() {
                                                return static_cast<bool>(toggle2) ? "Active" : "Inactive";
                                            },
                                            .fontSize = 14
                                        }
                                    }
                                },

                                Divider {},

                                // Radio Buttons Section
                                Text {
                                    .value = "Radio Buttons",
                                    .fontSize = 22,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color(0.15f, 0.15f, 0.15f, 1.0f),
                                    .padding = EdgeInsets(0, 0, 8, 0)
                                },

                                VStack {
                                    .spacing = 8,
                                    .alignItems = AlignItems::start,
                                    .children = {
                                        RadioButton {
                                            .selected = [&]() { return selectedOption.get() == "option1"; },
                                            .value = "option1",  // For reference only
                                            .label = "Option 1",
                                            .onChange = [&]() {
                                                selectedOption = "option1";
                                                std::cout << "Selected: Option 1" << std::endl;
                                            }
                                        },
                                        RadioButton {
                                            .selected = [&]() { return selectedOption.get() == "option2"; },
                                            .value = "option2",
                                            .label = "Option 2",
                                            .onChange = [&]() {
                                                selectedOption = "option2";
                                                std::cout << "Selected: Option 2" << std::endl;
                                            }
                                        },
                                        RadioButton {
                                            .selected = [&]() { return selectedOption.get() == "option3"; },
                                            .value = "option3",
                                            .label = "Option 3",
                                            .onChange = [&]() {
                                                selectedOption = "option3";
                                                std::cout << "Selected: Option 3" << std::endl;
                                            }
                                        }
                                    }
                                },

                                Divider {},

                                // Badges Section
                                Text {
                                    .value = "Badges",
                                    .fontSize = 22,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color(0.15f, 0.15f, 0.15f, 1.0f),
                                    .padding = EdgeInsets(0, 0, 8, 0)
                                },

                                HStack {
                                    .spacing = 15,
                                    .alignItems = AlignItems::center,
                                    .children = {
                                        Badge {
                                            .text = "New",
                                            .badgeColor = Colors::red
                                        },
                                        Badge {
                                            .text = "Pro",
                                            .badgeColor = Colors::blue
                                        },
                                        Badge {
                                            .text = "Beta",
                                            .badgeColor = Colors::yellow,
                                            .textColor = Colors::black
                                        },
                                        Badge {
                                            .text = "99+",
                                            .badgeColor = Colors::green
                                        }
                                    }
                                }
                            }
                        },

                        // Right column
                        VStack {
                            .spacing = 20,
                            .expansionBias = 1.0f,
                            .backgroundColor = Colors::white,
                            .padding = 20,
                            .cornerRadius = 8,
                            .borderColor = Color(0.85f, 0.85f, 0.85f, 1.0f),
                            .borderWidth = 1,
                            .children = {
                                // Progress Bars Section
                                Text {
                                    .value = "Progress Bars",
                                    .fontSize = 22,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color(0.15f, 0.15f, 0.15f, 1.0f),
                                    .padding = EdgeInsets(0, 0, 8, 0)
                                },

                                VStack {
                                    .spacing = 15,
                                    .children = {
                                        VStack {
                                            .spacing = 5,
                                            .children = {
                                                Text {
                                                    .value = "Determinate Progress",
                                                    .fontSize = 12,
                                                    .color = Colors::darkGray
                                                },
                                                ProgressBar {
                                                    .value = progress2,
                                                    .mode = ProgressBarMode::Determinate,
                                                    .showLabel = true
                                                }
                                            }
                                        },

                                        VStack {
                                            .spacing = 5,
                                            .children = {
                                                Text {
                                                    .value = "Indeterminate (Loading)",
                                                    .fontSize = 12,
                                                    .color = Colors::darkGray
                                                },
                                                ProgressBar {
                                                    .mode = ProgressBarMode::Indeterminate,
                                                    .fillColor = Colors::green
                                                }
                                            }
                                        },

                                        VStack {
                                            .spacing = 5,
                                            .children = {
                                                Text {
                                                    .value = "Custom Color",
                                                    .fontSize = 12,
                                                    .color = Colors::darkGray
                                                },
                                                ProgressBar {
                                                    .value = 0.85f,
                                                    .fillColor = Color::hex(0xFF5722),
                                                    .showLabel = true
                                                }
                                            }
                                        }
                                    }
                                },

                                Divider {},

                                // Sliders Section
                                Text {
                                    .value = "Interactive Sliders",
                                    .fontSize = 22,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color(0.15f, 0.15f, 0.15f, 1.0f),
                                    .padding = EdgeInsets(0, 0, 8, 0)
                                },

                                VStack {
                                    .spacing = 20,
                                    .children = {
                                        VStack {
                                            .spacing = 8,
                                            .children = {
                                                HStack {
                                                    .justifyContent = JustifyContent::spaceBetween,
                                                    .children = {
                                                        Text {
                                                            .value = "Volume",
                                                            .fontSize = 14,
                                                            .color = Colors::darkGray
                                                        },
                                                        Text {
                                                            .value = [&]() {
                                                                return std::format("{:.0f}%", static_cast<float>(sliderValue1) * 100);
                                                            },
                                                            .fontSize = 14,
                                                            .fontWeight = FontWeight::bold,
                                                            .color = Colors::blue
                                                        }
                                                    }
                                                },
                                                Slider {
                                                    .value = sliderValue1,
                                                    .minValue = 0.0f,
                                                    .maxValue = 1.0f,
                                                    .step = 0.01f,
                                                    .onChange = [&]() {
                                                        std::cout << "Slider 1: " << static_cast<float>(sliderValue1) << std::endl;
                                                    }
                                                }
                                            }
                                        },

                                        VStack {
                                            .spacing = 8,
                                            .children = {
                                                HStack {
                                                    .justifyContent = JustifyContent::spaceBetween,
                                                    .children = {
                                                        Text {
                                                            .value = "Temperature",
                                                            .fontSize = 14,
                                                            .color = Colors::darkGray
                                                        },
                                                        Text {
                                                            .value = [&]() {
                                                                return std::format("{:.0f}°C", static_cast<float>(sliderValue2));
                                                            },
                                                            .fontSize = 14,
                                                            .fontWeight = FontWeight::bold,
                                                            .color = Colors::red
                                                        }
                                                    }
                                                },
                                                Slider {
                                                    .value = sliderValue2,
                                                    .minValue = 0.0f,
                                                    .maxValue = 100.0f,
                                                    .step = 1.0f,
                                                    .activeColor = Colors::red
                                                }
                                            }
                                        },

                                        VStack {
                                            .spacing = 8,
                                            .children = {
                                                HStack {
                                                    .justifyContent = JustifyContent::spaceBetween,
                                                    .children = {
                                                        Text {
                                                            .value = "Brightness",
                                                            .fontSize = 14,
                                                            .color = Colors::darkGray
                                                        },
                                                        Text {
                                                            .value = [&]() {
                                                                return std::format("{:.0f}%", static_cast<float>(sliderValue3) * 100);
                                                            },
                                                            .fontSize = 14,
                                                            .fontWeight = FontWeight::bold,
                                                            .color = Colors::yellow
                                                        }
                                                    }
                                                },
                                                Slider {
                                                    .value = sliderValue3,
                                                    .minValue = 0.0f,
                                                    .maxValue = 1.0f,
                                                    .step = 0.05f,
                                                    .activeColor = Colors::yellow
                                                }
                                            }
                                        }
                                    }
                                },

                                Divider {},

                                // Buttons Section
                                Text {
                                    .value = "Buttons",
                                    .fontSize = 22,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color(0.15f, 0.15f, 0.15f, 1.0f),
                                    .padding = EdgeInsets(0, 0, 8, 0)
                                },

                                HStack {
                                    .spacing = 10,
                                    .children = {
                                        Button {
                                            .text = "Primary",
                                            .backgroundColor = Colors::blue,
                                            .padding = EdgeInsets(10, 20, 10, 20),
                                            .cornerRadius = 6,
                                            .onClick = [&]() {
                                                clickCount++;
                                                std::cout << "Primary button clicked! Count: " << static_cast<int>(clickCount) << std::endl;
                                            }
                                        },
                                        Button {
                                            .text = "Success",
                                            .backgroundColor = Colors::green,
                                            .padding = EdgeInsets(10, 20, 10, 20),
                                            .cornerRadius = 6,
                                            .onClick = [&]() {
                                                std::cout << "Success button clicked!" << std::endl;
                                            }
                                        },
                                        Button {
                                            .text = "Danger",
                                            .backgroundColor = Colors::red,
                                            .padding = EdgeInsets(10, 20, 10, 20),
                                            .cornerRadius = 6,
                                            .onClick = [&]() {
                                                std::cout << "Danger button clicked!" << std::endl;
                                            }
                                        }
                                    }
                                },

                                HStack {
                                    .spacing = 10,
                                    .alignItems = AlignItems::center,
                                    .children = {
                                        Text {
                                            .value = [&]() {
                                                return std::format("Clicks: {}", static_cast<int>(clickCount));
                                            },
                                            .fontSize = 14,
                                            .color = Colors::darkGray
                                        },
                                        Badge {
                                            .text = [&]() {
                                                return std::to_string(static_cast<int>(clickCount));
                                            },
                                            .badgeColor = Colors::blue
                                        }
                                    }
                                }
                            }
                        }
                    }
                },

                // Footer
                Divider {
                    .thickness = 2.0f,
                    .color = Colors::gray
                },

                HStack {
                    .justifyContent = JustifyContent::center,
                    .spacing = 10,
                    .children = {
                        Text {
                            .value = "Flux UI Framework",
                            .fontSize = 12,
                            .color = Colors::darkGray
                        },
                        Badge {
                            .text = "v1.0",
                            .badgeColor = Color::hex(0x9C27B0),
                            .fontSize = 10
                        }
                    }
                }
            }
        }
    );

    return app.exec();
}

