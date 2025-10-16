# Flexbox Layout Demo

This demo showcases the flexible layout capabilities of Flux UI using `expansionBias` and `compressionBias` properties.

## Features Demonstrated

### 1. Equal Expansion (expansionBias = 1.0)
Shows three components with equal `expansionBias` values, resulting in equal-sized boxes that fill the available space proportionally.

### 2. Different Expansion Ratios (1x, 2x, 1x)
Demonstrates how different `expansionBias` values create proportional sizing. The middle component takes twice as much space as the outer components.

### 3. Mixed Expansion (Fixed + Flexible)
Shows how fixed-size components (expansionBias = 0.0) work alongside flexible components (expansionBias = 1.0).

### 4. Compression Test (compressionBias)
Demonstrates how components shrink when space is limited, using different `compressionBias` values to control shrinking behavior.

### 5. Vertical Stack Expansion
Shows how VStack components can also use expansion to create equal-height columns with different content amounts.

## How to Run

```bash
# Build the demo
cd /path/to/flux/build
make flexbox_demo

# Run with HTML canvas output (recommended for visual results)
FLUX_USE_HTML_CANVAS=1 ./flexbox_demo

# View the output
open flux_output.html
```

## Key Concepts

### expansionBias (default: 0.0)
- Controls how much a component grows when extra space is available
- `0.0` = Component stays at its preferred size (no growth)
- `1.0` = Component takes an equal share of available space
- Higher values = Proportionally more space

### compressionBias (default: 1.0)
- Controls how much a component shrinks when space is limited
- `0.0` = Component never shrinks below its preferred size (rigid)
- `1.0` = Component shrinks proportionally with others
- Higher values = Shrinks more when space is constrained

## CSS Flexbox Equivalents

| Flux Property | CSS Equivalent | Purpose |
|---------------|----------------|---------|
| `expansionBias` | `flex-grow` | How much to grow when space is available |
| `compressionBias` | `flex-shrink` | How much to shrink when space is limited |
| `preferredSize()` | `flex-basis` | Initial size before growing/shrinking |

## Algorithm

The layout follows CSS flexbox principles:

1. **If any child has `expansionBias > 0`**: Distribute ALL available space proportionally
2. **If no expansion**: Use preferred sizes, but apply compression if needed
3. **Compression**: Shrink based on `preferredSize × compressionBias` weights
