#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/View.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace flux {

class RenderContext;
class PlatformWindow; // Forward declaration
struct LayoutNode;

struct WindowConfig {
    Size size = {1280, 720};
    std::string title = "Flux Application";
    bool fullscreen = false;
    bool resizable = true;
};

class Window {
private:
    std::unique_ptr<ImmediateModeRenderer> renderer_;
    std::unique_ptr<RenderContext> renderContext_; // Owned by PlatformWindow now
    View rootView_;
    WindowConfig config_;
    Size currentSize_;

    std::unique_ptr<PlatformWindow> platformWindow_; // Wayland window implementation

    // Keyboard state tracking
    KeyModifier currentModifiers_;
    
    // Pending keyboard events (to be dispatched during render frame)
    std::vector<KeyEvent> pendingKeyDownEvents_;
    std::vector<KeyEvent> pendingKeyUpEvents_;
    std::vector<TextInputEvent> pendingTextInputEvents_;

    // Focus management state (previously in FocusManager)
    struct FocusableViewInfo {
        View* view;
        Rect bounds;
        std::string key;  // Unique key for this view (explicit or auto-generated)
    };
    std::string focusedKey_;  // Track focus by key for stability across frames
    std::vector<FocusableViewInfo> focusableViews_;

public:
    explicit Window(const WindowConfig& config);
    ~Window();

    void setRootView(View component);
    void render();
    void resize(const Size& newSize);
    void setFullscreen(bool fullscreen);
    void setTitle(const std::string& title);
    unsigned int windowID() const; // Returns the platform-specific window ID

    // Event handlers
    void handleMouseMove(float x, float y);
    void handleMouseDown(int button, float x, float y);
    void handleMouseUp(int button, float x, float y);
    void handleKeyDown(int key);
    void handleKeyUp(int key);
    void handleTextInput(const std::string& text);
    void handleResize(const Size& newSize);
    
    // Keyboard event dispatch
    void dispatchKeyDown(const KeyEvent& event);
    void dispatchKeyUp(const KeyEvent& event);
    void dispatchTextInput(const TextInputEvent& event);

    // Event dispatch to renderer
    void dispatchEvent(const Event& event);

    // Cursor management
    void setCursor(CursorType cursor);
    CursorType currentCursor() const;

    // Platform-specific accessors (for Application event handling)
    PlatformWindow* platformWindow() { return platformWindow_.get(); }
    
    // Focus management (formerly in FocusManager)
    void registerFocusableView(View* view, const Rect& bounds);
    void clearFocusableViews();
    void focusNext();
    void focusPrevious();
    View* getFocusedView() const;
    void clearFocus();
    bool focusViewAtPoint(const Point& point);
    size_t getFocusableViewCount() const { return focusableViews_.size(); }
    std::string getFocusedKey() const { return focusedKey_; }
    static View* findViewByKey(LayoutNode& root, const std::string& key);
    bool dispatchKeyDownToFocused(LayoutNode& root, const KeyEvent& event);
    bool dispatchKeyUpToFocused(LayoutNode& root, const KeyEvent& event);
    bool dispatchTextInputToFocused(LayoutNode& root, const TextInputEvent& event);
    
    // Process pending keyboard events (called during render frame with layout tree)
    void processPendingEvents(LayoutNode& layoutTree);
    
private:
    // Global keyboard shortcut handlers
    bool handleGlobalShortcut(const KeyEvent& event);
    
    // Update keyboard modifier state
    void updateModifiers(int key, bool pressed);

    // Focus management helpers (formerly in FocusManager)
    int findViewIndexByKey(const std::string& key) const;
    std::string generateAutoKey(const View* view, int registrationIndex) const;
};

} // namespace flux
