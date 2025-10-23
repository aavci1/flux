# Component Guide

## Standardized View Creation

All views in Flux follow a consistent pattern with three key methods: `layout()`, `body()`, and `render()`. This standard makes views easy to understand, predictable, and maintainable.

## The Three Methods

### 1. `layout()` - Positions Children (Container Views Only)

**Purpose**: Calculate and store the bounds for each child.

**When to implement**: Only when the view changes the bounds of its children (like `VStack`, `HStack`).

**Default behavior**: For leaf views (Text, Button, etc.), just pass through the bounds.

```cpp
// Container view (VStack, HStack) - implements custom layout
LayoutInfo layout(const Rect& bounds) const {
    // Calculate child positions
    // Store bounds in mutable cache
    // Call layout() on each child
    return LayoutInfo(bounds, preferredSize());
}

// Leaf view (Text, Button, etc.) - simple passthrough
LayoutInfo layout(const Rect& bounds) const {
    return LayoutInfo(bounds, preferredSize());
}
```

### 2. `body()` - Returns Children

**Purpose**: Return the children of this view for composite view pattern (views that create new child views).

**When to implement**: Always - it's part of the view contract.

**Default behavior**:
- Composite views: return their created children (views that generate children from their properties)
- Layout containers: return empty (VStack, HStack just position children, they don't create new ones)
- Leaf views: return empty vector

```cpp
// Composite view - returns generated children
View body() const {
    // Example: A Card view that creates Text children from properties
    return VStack {
        .children = {
            Text{.value = title},
            Text{.value = subtitle}
        }
    };
}

// Layout container - returns empty (doesn't create children, just arranges them)
View body() const {
    return View{};
}

// Leaf view - returns empty
View body() const {
    return View{};
}
```

### 3. `render()` - Draws the View

**Purpose**: Render the view using the unified helper.

**Implementation Pattern**:
```cpp
void render(RenderContext& ctx, const LayoutInfo& layout) const {
    ViewHelpers::renderView(*this, ctx, layout);

    // Custom drawing code here (if needed)
    ctx.drawText(value, position, fontSize, color);
}
```

**The helper automatically**:
- Applies transforms (offset, rotation, scale, opacity)
- Draws shadow, background, and border from standard properties
- Renders children using cached bounds from `layout()`
- Handles `ctx.save()` and `ctx.restore()`

**All views follow the same pattern**:
- Layout containers (VStack, HStack) cache child bounds in `layout()`
- Leaf views (Text, Button) have no children to cache
- Composite views generate children in `body()` and cache them in `layout()`

## View Categories

### 1. Leaf Views (No Children)

Examples: `Text`, `Button`, `Spacer`, `Canvas`

**Characteristics**:
- Simple `layout()` - just pass through bounds
- `body()` returns empty vector
- `drawContent()` provides custom rendering
- `render()` uses unified helper

```cpp
struct MyLeafView {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> text;

    // layout() - Simple passthrough
    LayoutInfo layout(const Rect& bounds) const {
        return LayoutInfo(bounds, preferredSize());
    }

    // body() - No children
    View body() const {
        return View{};
    }

    // render() - Apply decorations and draw custom content
    void render(RenderContext& ctx, const LayoutInfo& layout) const {
        ViewHelpers::renderView(*this, ctx, layout);

        // Draw your custom content here
        ctx.drawText(static_cast<std::string>(text), ...);

        // You can draw anything: shapes, images, animations, etc.
        ctx.drawCircle(center, radius, color);
        ctx.drawLine(start, end, width, strokeColor);
    }

    Size preferredSize() const {
        return {200, 100};
    }
};
```

### 2. Layout Container Views

Examples: `VStack`, `HStack`

**Characteristics**:
- Custom `layout()` - positions children and caches bounds
- `body()` returns children with cached layout applied
- `render()` uses unified helper

```cpp
struct MyLayoutContainer {
    FLUX_VIEW_PROPERTIES;

    Property<std::vector<View>> children = {};
    mutable std::vector<Rect> childBounds_ = {}; // Cache

    // layout() - Position children
    LayoutInfo layout(const Rect& bounds) const {
        childBounds_.clear();

        // Calculate positions for each child
        std::vector<View> childrenVec = children;
        for (auto& child : childrenVec) {
            Rect childRect = /* calculate position */;
            childBounds_.push_back(childRect);
            child.layout(childRect); // Layout the child
        }

        return LayoutInfo(bounds, preferredSize());
    }

    // body() - Returns children with cached layout applied
    View body() const {
        // For layout containers, we typically don't use body() since
        // they handle children through their children property directly
        return View{};
    }

    // render() - Apply decorations and render children with cached bounds
    void render(RenderContext& ctx, const LayoutInfo& layout) const {
        ViewHelpers::renderView(*this, ctx, layout);
        ViewHelpers::renderChildrenWithCachedBounds(ctx, children, childBounds_);
    }

    Size preferredSize() const {
        // Calculate based on children
        return {width, height};
    }
};
```

### 3. Composite Views (Generate Children)

Examples: Custom views that create their own child hierarchy from properties

**Characteristics**:
- Simple `layout()` - usually passthrough
- `body()` returns generated children
- `render()` uses unified helper (children from `body()` are automatically rendered)

```cpp
struct Card {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> title;
    Property<std::string> subtitle;
    Property<Color> accentColor = ColorShortcut::primary;

    // layout() - Simple passthrough
    LayoutInfo layout(const Rect& bounds) const {
        return LayoutInfo(bounds, preferredSize());
    }

    // body() - Creates child views from properties
    View body() const {
        return VStack {
            .padding = EdgeInsets{20},
            .spacing = 10,
            .children = {
                Text {
                    .value = static_cast<std::string>(title),
                    .fontSize = 24,
                    .fontWeight = FontWeight::bold,
                    .color = accentColor
                },
                Text {
                    .value = static_cast<std::string>(subtitle),
                    .fontSize = 16,
                    .color = ColorShortcut::gray
                }
            }
        };
    }

    // render() - Apply decorations and render children from body()
    void render(RenderContext& ctx, const LayoutInfo& layout) const {
        ViewHelpers::renderView(*this, ctx, layout);
        View child = body();
        if (child) {
            child.render(ctx, layout);
        }
    }

    Size preferredSize() const {
        // Calculate from children
        View child = body();
        if (child) {
            return child.preferredSize();
        }
        return {300, 100};
    }
};
```

## Summary of View Types

All views use the `ViewHelpers::renderView()` helper for decorations, then handle their own content:

1. **Leaf Views** - Draw custom content directly (Text, Button, Spacer)
   - `layout()`: Simple passthrough
   - `body()`: Returns empty
   - `render()`: `ViewHelpers::renderView()` + custom drawing code

   **Custom Drawing**: Create your own components by implementing the three-method pattern and putting custom drawing code directly in `render()` after the helper call. See the Stack Alignment Demo (`examples/02-stack-alignment-demo/`) for custom layout components and the Dashboard example (`examples/08-dashboard/`) for comprehensive chart components.

2. **Layout Containers** - Position children (VStack, HStack)
   - `layout()`: Calculate and cache child positions
   - `body()`: Returns children (simple helper)
   - `render()`: `ViewHelpers::renderView()` (children automatically rendered with cached bounds)

3. **Composite Views** - Create child hierarchy from properties (Card, custom components)
   - `layout()`: Generate children and cache their layout
   - `body()`: Generate and return children
   - `render()`: `ViewHelpers::renderView()` (children automatically rendered with cached bounds)

**Key Benefits**:
- **Unified approach**: All views use `ViewHelpers::renderView()` for everything
- **Automatic children**: Helper automatically renders children using cached bounds from `layout()`
- **Clear responsibility**: Views cache child bounds in `layout()`, helper renders them in `render()`
- **Developer friendly**: Single render implementation, no manual child rendering
- **Maintainable**: All rendering logic centralized in the helper

## RenderContext API

### Drawing Primitives

```cpp
// Rectangles
ctx.drawRect(rect, color);
ctx.drawRoundedRect(rect, radius, color);
ctx.drawRoundedRectBorder(rect, radius, color, width);

// Circles
ctx.drawCircle(center, radius, color);

// Lines
ctx.drawLine(start, end, width, color);

// Text
ctx.drawText(text, position, fontSize, color, weight);
Size size = ctx.measureText(text, fontSize, weight);

// Property management
ctx.save();     // Save transform state
ctx.restore();  // Restore transform state
ctx.translate(x, y);
ctx.rotate(angle);
ctx.scale(sx, sy);
ctx.clipRect(rect);

// Frame control
ctx.clear(color);
ctx.present();
```

## Key Benefits of This Standard

### 1. **Separation of Concerns**
- `layout()` only handles positioning logic
- `render()` only handles drawing
- No duplicate code between the two

### 2. **Predictable Behavior**
- Every view follows the same pattern
- Easy to understand at a glance whether a view is a container or leaf
- Cached bounds eliminate duplicate calculations

### 3. **Composability**
- `body()` enables composite view patterns
- Easy to create views that are composed of other views
- Clear parent-child relationships

### 4. **Maintainability**
- Helper functions (`applyViewDecorations`) reduce boilerplate
- Changes to common behavior can be made in one place
- Clear documentation of what each method does

## Property Usage

Properties allow views to be configured by users:

```cpp
struct MyView {
    FLUX_VIEW_PROPERTIES;  // Standard view properties

    Property<std::string> title;           // User sets this
    Property<int> count = 0;               // Default value
    Property<Color> color = ColorShortcut::primary;  // Color shortcut
};
```

In render methods, evaluate properties using implicit conversion:

```cpp
void render(RenderContext& ctx, const LayoutInfo& layout) const {
    std::string titleText = title;    // Evaluates property
    int countValue = count;           // Gets current value
    Color bgColor = resolveColor(color);  // Resolves color/shortcut
}
```

Users can set properties with values, state references, or lambdas:

```cpp
MyView {
    .title = "Static Text",              // Direct value
    .title = username,                   // Stateful Property reference
    .title = [&]() { return name; }     // Lambda (computed)
}
```

## Internal State

Views can have internal state for interaction (hover, pressed, etc.):

```cpp
struct Button {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> text;
    std::function<void()> onClick;

    // Internal state - mutable for const methods
    mutable bool isHovered = false;
    mutable bool isPressed = false;

    void render(RenderContext& ctx, const LayoutInfo& layout) const {
        // Use internal state to determine appearance
        Color bgColor = isHovered ?
            resolveColor(color).darken(0.1) :
            resolveColor(color);
        // ...
    }
};
```

## Best Practices

### ✓ DO: Follow the Standard Pattern

Always implement all three methods: `layout()`, `body()`, and `render()`.

### ✓ DO: Use Mutable Cache for Layout

Container views should cache child bounds in a mutable member to avoid recalculating in render().

### ✓ DO: Use Helper Functions

Use `applyViewDecorations()` to reduce boilerplate in render methods.

### ✓ DO: Document Intent

Add comments explaining what each method does, especially if the implementation is non-trivial.

### ✗ DON'T: Duplicate Layout Logic

Never recalculate child positions in both `layout()` and `render()`. Calculate once in `layout()`, cache, and reuse in `render()`.

### ✗ DON'T: Skip Methods

Always implement all three methods, even if `body()` returns an empty vector or `layout()` is a simple passthrough.

## Built-in Components

### Leaf Views

#### Text
Displays text with formatting and alignment.

**Properties**: `value`, `fontSize`, `fontWeight`, `color`, `textAlign`, `verticalAlign`, `lineHeight`

**Type**: Leaf view (no children)

#### Button
Interactive button with multiple styles and states.

**Properties**: `text`, `style`, `disabled`, `color`

**Internal State**: `isHovered`, `isPressed`

**Callbacks**: `onClick`

**Type**: Leaf view (no children)

#### Spacer
Invisible view that fills available space.

**Properties**: `minLength`

**Type**: Leaf view (no children)

### Container Views

#### VStack
Arranges children vertically with spacing.

**Properties**: `children`, `spacing`, `alignment`

**Layout**: Distributes height equally among children

**Type**: Container view

#### HStack
Arranges children horizontally with spacing.

**Properties**: `children`, `spacing`, `alignment`, `crossAlignment`

**Layout**: Distributes width equally among children

**Type**: Container view

### Flexible Layout with expansionBias and compressionBias

All views have two properties that control how they grow and shrink within their parent container:

#### expansionBias (default: 0.0)
Controls how much a view grows relative to its siblings when extra space is available.

- `0.0` - View stays at its `preferredSize()` (doesn't grow)
- `1.0` - View takes an equal share of extra space with other views that have `expansionBias = 1.0`
- `2.0` - View takes twice as much extra space as views with `expansionBias = 1.0`

```cpp
HStack {
    .children = {
        Text {
            .value = "Fixed",
            .expansionBias = 0.0  // Stays at preferred width
        },
        Text {
            .value = "Flexible",
            .expansionBias = 1.0  // Fills remaining space
        },
        Spacer {
            // Spacer has expansionBias = 1.0 by default
        }
    }
}
```

#### compressionBias (default: 1.0)
Controls how much a view shrinks relative to its siblings when space is limited.

- `0.0` - View never shrinks below its `preferredSize()` (rigid)
- `1.0` - View shrinks proportionally with other views
- `2.0` - View shrinks twice as much as views with `compressionBias = 1.0`

```cpp
HStack {
    .children = {
        Text {
            .value = "Important",
            .compressionBias = 0.0  // Never shrinks
        },
        Text {
            .value = "Less important",
            .compressionBias = 2.0  // Shrinks more if needed
        }
    }
}
```

#### Common Patterns

**Fixed Size Component:**
```cpp
Text {
    .value = "Title",
    .expansionBias = 0.0,    // Don't grow
    .compressionBias = 0.0   // Don't shrink
}
```

**Fill Available Space:**
```cpp
Text {
    .value = "Content",
    .expansionBias = 1.0,    // Grow to fill
    .compressionBias = 1.0   // Can shrink if needed
}
```

**Spacer (default behavior):**
```cpp
Spacer {
    // expansionBias = 1.0 by default
    // compressionBias = 1.0 by default
    // Fills extra space and shrinks when needed
}
```

**Priority Layout:**
```cpp
HStack {
    .children = {
        Text { .value = "High priority", .expansionBias = 2.0 },
        Text { .value = "Low priority", .expansionBias = 1.0 }
    }
}
// High priority text gets 2x the extra space
```

#### How It Works

The layout algorithm for HStack and VStack:

1. **Start with base sizes**: Each child starts at its `preferredSize()`
2. **Calculate remaining space**: `availableSpace - sum(preferredSizes) - spacing`
3. **If extra space (positive)**:
   - Sum all `expansionBias` values
   - Distribute extra space proportionally: `child gets (extraSpace × child.expansionBias / totalExpansionBias)`
4. **If deficit (negative)**:
   - Sum all `compressionBias` values
   - Shrink proportionally: `child shrinks by (deficit × child.compressionBias / totalCompressionBias)`
   - Children never shrink below 0

This gives you precise control over how space is distributed, similar to CSS flexbox's `flex-grow` and `flex-shrink`, but with more intuitive naming for the Flux framework.

## Summary

The standardized view creation pattern in Flux makes it easy to create custom views:

1. **Define your struct** with `FLUX_VIEW_PROPERTIES`
2. **Add properties** for user configuration
3. **Implement `layout()`** - simple passthrough for leaf views, custom positioning for containers
4. **Implement `body()`** - return empty for leaf views, return children for containers
5. **Implement `render()`** - use `applyViewDecorations()` then draw custom content or children
6. **Implement `preferredSize()`** - return natural size

The three-method pattern ensures:
- ✓ Clear separation between layout and rendering
- ✓ No duplicate code
- ✓ Predictable, maintainable views
- ✓ Support for composition through `body()`

## Quick Reference

```cpp
// Leaf View Template (with custom drawing)
struct MyLeafView {
    FLUX_VIEW_PROPERTIES;
    Property<std::string> text;

    LayoutInfo layout(const Rect& bounds) const {
        return LayoutInfo(bounds, preferredSize());
    }

    View body() const { return View{}; }

    void render(RenderContext& ctx, const LayoutInfo& layout) const {
        ViewHelpers::renderView(*this, ctx, layout);
        /* custom drawing here */
    }

    Size preferredSize() const { /* calculate size */ }
};

// Layout Container Template (VStack, HStack)
struct MyLayoutContainer {
    FLUX_VIEW_PROPERTIES;
    Property<std::vector<View>> children = {};
    mutable std::vector<Rect> childBounds_ = {};

    LayoutInfo layout(const Rect& bounds) const {
        /* position children, cache bounds */
    }

    View body() const {
        // Layout containers typically don't use body() since they handle
        // children through their children property directly
        return View{};
    }

    void render(RenderContext& ctx, const LayoutInfo& layout) const {
        ViewHelpers::renderView(*this, ctx, layout);
    }

    Size preferredSize() const { /* calculate from children */ }
};

// Composite View Template
struct MyCompositeView {
    FLUX_VIEW_PROPERTIES;
    Property<std::string> title;

    LayoutInfo layout(const Rect& bounds) const {
        return LayoutInfo(bounds, preferredSize());
    }

    View body() const {
        return /* generated child view */;
    }

    void render(RenderContext& ctx, const LayoutInfo& layout) const {
        ViewHelpers::renderView(*this, ctx, layout);
    }

    Size preferredSize() const { /* calculate from children */ }
};
```

