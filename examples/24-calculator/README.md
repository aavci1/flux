# Calculator Example

A simple calculator application built with the Flux UI framework, demonstrating state management, reactive UI updates, and complex layout design.

## Features

### Basic Operations
- **Addition (+)**: Add two numbers
- **Subtraction (-)**: Subtract two numbers  
- **Multiplication (×)**: Multiply two numbers
- **Division (÷)**: Divide two numbers with zero-division protection

### Special Functions
- **Clear (C)**: Reset calculator to initial state
- **Plus/Minus (±)**: Toggle sign of current number
- **Percentage (%)**: Convert current number to percentage (divide by 100)
- **Decimal Point (.)**: Add decimal point to current number
- **Equals (=)**: Perform calculation and display result

### UI Design
- **Display**: Large, right-aligned text showing current number/result
- **Button Grid**: 4×5 grid layout with proper spacing and colspan support
- **Color Coding**: 
  - Red buttons for operations (C, ±, %, ÷, ×, -, +, =)
  - Dark gray buttons for numbers (0-9)
  - Light gray for special functions
- **Grid Layout**: Uses Flux's Grid component with colspan for the "0" button spanning 2 columns

## Technical Implementation

### State Management
The calculator uses Flux's `State` system for reactive updates:

```cpp
State<std::string> display = "0";           // Current display value
State<std::string> operation = "";           // Current operation (+, -, ×, ÷)
State<double> firstNumber = 0.0;             // First operand
State<bool> waitingForOperand = false;       // Whether waiting for second operand
State<bool> shouldResetDisplay = false;      // Whether to reset display on next input
```

### Key Concepts Demonstrated

1. **Reactive UI**: Display updates automatically when state changes using lambda properties:
   ```cpp
   Text {
       .value = [&]() { return static_cast<std::string>(display); }
   }
   ```

2. **Complex State Logic**: Multiple interdependent state variables manage calculator behavior

3. **Event Handling**: Button clicks trigger state updates through lambda functions

4. **Layout Design**: 
   - `VStack` for vertical arrangement of display and button grid
   - `Grid` for button layout with 4 columns and 5 rows
   - `colspan` property for the "0" button spanning 2 columns
   - Proper spacing and padding throughout

5. **Error Handling**: Division by zero protection with error display

6. **Number Formatting**: Smart formatting for integers vs. decimals

### Calculator Logic Flow

1. **Number Input**: Updates display, clears operation waiting state
2. **Operation Input**: Stores first number and operation, prepares for second operand
3. **Calculation**: Performs operation when equals is pressed or new operation is selected
4. **Display Update**: Formats and shows result, manages display reset logic

## Building and Running

```bash
# Build the project
mkdir build && cd build
cmake ..
make calculator

# Run the calculator
./calculator
```

## Usage

1. **Basic Calculation**: Enter first number → operation → second number → equals
2. **Chained Operations**: Perform multiple operations in sequence
3. **Clear**: Press 'C' to reset calculator
4. **Decimal Numbers**: Use '.' button to add decimal points
5. **Negative Numbers**: Use '±' to toggle sign
6. **Percentages**: Use '%' to convert to percentage

## Design Patterns

This example demonstrates several important Flux patterns:

- **State-driven UI**: All UI updates driven by state changes
- **Lambda Properties**: Reactive text updates using lambda functions
- **Event Handling**: Clean separation of UI and business logic
- **Layout Composition**: Building complex layouts using Grid with colspan/rowspan
- **Error Handling**: Graceful handling of edge cases (division by zero)

## Customization

The calculator can be easily customized:

- **Colors**: Modify button colors by changing `backgroundColor` properties
- **Size**: Adjust window size and button dimensions
- **Operations**: Add more mathematical functions (sqrt, power, etc.)
- **Layout**: Modify grid dimensions, colspan/rowspan values, or button arrangement
- **Styling**: Customize fonts, borders, and visual effects
