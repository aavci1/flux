#include <Flux.hpp>
#include <string>
#include <cmath>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // Calculator state
    Property<std::string> display = std::string("0");
    Property<std::string> operation = std::string("");
    Property<double> firstNumber = 0.0;
    Property<bool> waitingForOperand = false;
    Property<bool> shouldResetDisplay = false;

    // Helper function to update display
    auto updateDisplay = [&](const std::string& value) {
        display = value;
    };

    // Helper function to clear calculator
    auto clearCalculator = [&]() {
        display = std::string("0");
        operation = std::string("");
        firstNumber = 0.0;
        waitingForOperand = false;
        shouldResetDisplay = false;
    };

    // Helper function to perform calculation
    auto performCalculation = [&]() {
        if (static_cast<std::string>(operation).empty()) return;
        
        double secondNumber = std::stod(static_cast<std::string>(display));
        double result = 0.0;
        
        if (operation == "+") {
            result = firstNumber + secondNumber;
        } else if (operation == "-") {
            result = firstNumber - secondNumber;
        } else if (operation == "×") {
            result = firstNumber * secondNumber;
        } else if (operation == "÷") {
            if (secondNumber != 0) {
                result = firstNumber / secondNumber;
            } else {
                updateDisplay("Error");
                return;
            }
        }
        
        // Format result
        if (result == std::floor(result) && result < 1e10) {
            updateDisplay(std::format("{:.0f}", result));
        } else {
            updateDisplay(std::format("{:.8g}", result));
        }
        
        operation = std::string("");
        waitingForOperand = true;
        shouldResetDisplay = true;
    };

    // Helper function to handle number input
    auto handleNumberInput = [&](const std::string& number) {
        if (shouldResetDisplay || static_cast<std::string>(display) == "0") {
            updateDisplay(number);
            shouldResetDisplay = false;
        } else {
            updateDisplay(static_cast<std::string>(display) + number);
        }
        waitingForOperand = false;
    };

    // Helper function to handle operation input
    auto handleOperationInput = [&](const std::string& op) {
        if (!static_cast<std::string>(operation).empty() && !waitingForOperand) {
            performCalculation();
        }
        
        firstNumber = std::stod(static_cast<std::string>(display));
        operation = op;
        waitingForOperand = true;
        shouldResetDisplay = true;
    };

    Window window({
        .size = {320, 480},
        .title = "Calculator"
    });

    window.setRootView(
        VStack {
            .padding = EdgeInsets{16},
            .spacing = 12,
            .children = {
                // Display
                Text {
                    .value = [&]() { return static_cast<std::string>(display); },
                    .fontSize = 32,
                    .fontWeight = FontWeight::medium,
                    .color = Colors::black,
                    .horizontalAlignment = HorizontalAlignment::trailing,
                    .padding = EdgeInsets{16, 20},
                    .backgroundColor = Colors::lightGray,
                    .cornerRadius = 8
                },
                
                // Button grid - 4 columns x 5 rows
                Grid {
                    .columns = 4,
                    .rows = 5,
                    .spacing = 8,
                    .expansionBias = 1,
                    .children = {
                        // Row 1: Clear, +/-, %, ÷
                        Button {
                            .text = "C",
                            .onClick = [&]() { clearCalculator(); },
                            .backgroundColor = Colors::red
                        },
                        Button {
                            .text = "±",
                            .onClick = [&]() {
                                if (static_cast<std::string>(display) != "0") {
                                    double value = std::stod(static_cast<std::string>(display));
                                    updateDisplay(std::format("{}", -value));
                                }
                            },
                            .backgroundColor = Colors::lightGray
                        },
                        Button {
                            .text = "%",
                            .onClick = [&]() {
                                double value = std::stod(static_cast<std::string>(display));
                                updateDisplay(std::format("{}", value / 100));
                            },
                            .backgroundColor = Colors::lightGray
                        },
                        Button {
                            .text = "÷",
                            .onClick = [&]() { handleOperationInput("÷"); },
                            .backgroundColor = Colors::red
                        },
                        
                        // Row 2: 7, 8, 9, ×
                        Button {
                            .text = "7",
                            .onClick = [&]() { handleNumberInput("7"); },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = "8",
                            .onClick = [&]() { handleNumberInput("8"); },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = "9",
                            .onClick = [&]() { handleNumberInput("9"); },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = "×",
                            .onClick = [&]() { handleOperationInput("×"); },
                            .backgroundColor = Colors::red
                        },
                        
                        // Row 3: 4, 5, 6, -
                        Button {
                            .text = "4",
                            .onClick = [&]() { handleNumberInput("4"); },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = "5",
                            .onClick = [&]() { handleNumberInput("5"); },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = "6",
                            .onClick = [&]() { handleNumberInput("6"); },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = "-",
                            .onClick = [&]() { handleOperationInput("-"); },
                            .backgroundColor = Colors::red
                        },
                        
                        // Row 4: 1, 2, 3, +
                        Button {
                            .text = "1",
                            .onClick = [&]() { handleNumberInput("1"); },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = "2",
                            .onClick = [&]() { handleNumberInput("2"); },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = "3",
                            .onClick = [&]() { handleNumberInput("3"); },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = "+",
                            .onClick = [&]() { handleOperationInput("+"); },
                            .backgroundColor = Colors::red
                        },
                        
                        // Row 5: 0 (spans 2 columns), ., =
                        Button {
                            .text = "0",
                            .colspan = 2,
                            .onClick = [&]() { handleNumberInput("0"); },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = ".",
                            .onClick = [&]() {
                                if (static_cast<std::string>(display).find('.') == std::string::npos) {
                                    if (shouldResetDisplay) {
                                        updateDisplay("0.");
                                        shouldResetDisplay = false;
                                    } else {
                                        updateDisplay(static_cast<std::string>(display) + ".");
                                    }
                                }
                            },
                            .backgroundColor = Colors::darkGray
                        },
                        Button {
                            .text = "=",
                            .onClick = [&]() { performCalculation(); },
                            .backgroundColor = Colors::red
                        }
                    }
                }
            }
        }
    );

    return app.exec();
}
