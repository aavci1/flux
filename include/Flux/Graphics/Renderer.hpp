#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>

namespace flux {

class Renderer {
private:
    RenderContext* renderContext_;
    View rootView_;

public:
    Renderer(RenderContext* ctx)
        : renderContext_(ctx), rootView_() {}

    Renderer(RenderContext* ctx, View component)
        : renderContext_(ctx), rootView_(std::move(component)) {}

    void renderFrame(const Rect& bounds) {
        if (!renderContext_) return;

        // Begin the frame
        renderContext_->beginFrame();

        // Clear the frame buffer
        renderContext_->clear(Color(1, 1, 1, 1));

        // Build layout tree and render using framework approach
        if (rootView_.operator->()) {
            // Layout the root view with RenderContext for accurate measurements
            LayoutNode layoutTree = rootView_->layout(*renderContext_, bounds);

            // Debug: Print layout tree before rendering
            std::cout << "\n=== Layout Tree ===" << std::endl;
            printLayoutTree(layoutTree);
            std::cout << "==================\n" << std::endl;

            // Render the tree by traversing it
            renderTree(layoutTree);
        }

        // Present the frame
        renderContext_->present();
    }

    void handleEvent(const struct Event& event) {
        // Handle input events by rebuilding UI if needed
        // State changes will automatically trigger redraws
    }

    void setRootView(View component) {
        rootView_ = std::move(component);
    }

private:
    void renderTree(const LayoutNode& node, Point parentOrigin = {0, 0}) {
        // Save the current rendering state
        renderContext_->save();

        // Calculate position relative to parent
        float relX = node.bounds.x - parentOrigin.x;
        float relY = node.bounds.y - parentOrigin.y;

        // Translate coordinate system to the view's position
        renderContext_->translate(relX, relY);

        // Create local bounds (view renders in its own coordinate system)
        Rect localBounds = {0, 0, node.bounds.width, node.bounds.height};

        // Clip rendering to the view's bounds if the view has clipping enabled
        if (node.view.shouldClip()) {
            Path clipPath;
            clipPath.rect(localBounds);
            renderContext_->clipPath(clipPath);
        }

        // Render the view with local coordinates
        node.view->render(*renderContext_, localBounds);

        // Recursively render children with current view's position as their parent origin
        Point currentOrigin = {node.bounds.x, node.bounds.y};
        for (const auto& child : node.children) {
            renderTree(child, currentOrigin);
        }

        // Restore the rendering state
        renderContext_->restore();
    }
};

// Alias for clarity
using ImmediateModeRenderer = Renderer;

struct Event {
    enum Type {
        MouseMove,
        MouseButton,
        KeyPress,
        KeyRelease,
        TextInput
    };

    Type type;
    union {
        struct { float x, y; } mouse;
        struct { int button; bool pressed; } mouseButton;
        struct { int key; bool pressed; } keyboard;
    };
    std::string text; // For TextInput events
};

} // namespace flux
