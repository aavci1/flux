# Why Choose Flux?

Flux is a modern C++ UI framework that embraces simplicity, directness, and the power of contemporary C++. Here's why developers are choosing Flux for their UI development needs.

## Pure C++ Philosophy

Flux is built on a simple principle: **everything is C++**. No separate markup languages, no foreign DSLs, no context switching. Just clean, modern C++ code.

```cpp
#include <Flux.hpp>
using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);
    
    Window window({
        .size = {400, 400},
        .title = "Flux App"
    });
    
    window.setRootView(
        Text {
            .fontSize = 48,
            .value = "Hello, world!"
        }
    );
    
    return app.exec();
}
```

### What This Means For You

**Single Language Development**
- Write your UI and logic in the same language
- No need to learn multiple languages or paradigms
- Debug everything with standard C++ tools
- One skill set to master

**Your Existing Tools Just Work**
- Standard C++ IDE features (IntelliSense, autocomplete) work perfectly
- clang-tidy, clang-format, and other C++ tools work seamlessly
- Refactoring tools understand your entire codebase
- No special IDE plugins or extensions required

## Modern C++ at Its Best

Flux leverages C++20 features to provide elegant, readable syntax that feels natural to modern C++ developers.

**Designated Initializers**
```cpp
Window window({
    .size = {800, 600},
    .title = "My App",
    .resizable = true
});
```

**Aggregate Initialization**
```cpp
Text {
    .fontSize = 24,
    .value = "Clean and readable",
    .color = Colors::blue
}
```

**Type Safety Throughout**
- Full compile-time type checking
- No dynamic typing or runtime type errors
- Your compiler catches mistakes early
- IDE knows exactly what properties are available

## Build Simplicity

Flux keeps your build process straightforward and fast.

**Standard C++ Compilation**
- No code generation steps
- No DSL compilation phase
- No resource packing or meta-object compilation
- Just straightforward C++ compilation

**Fast Incremental Builds**
- Change C++ code, recompile, done
- No intermediate compilation steps
- No generated code to rebuild
- Faster iteration cycles

**Simple CMake Integration**
```cmake
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE flux)
```

That's it. No special build plugins, no custom compilation steps, no resource systems.

## Unified Development Experience

### Seamless Refactoring

Everything is C++ code, so refactoring is effortless:
- Rename a variable: works everywhere
- Extract a function: standard C++ refactoring
- Move code between files: no cross-language concerns
- Change signatures: compiler finds all call sites

### Single-File Simplicity

For rapid prototyping or simple applications, put everything in one file:

```cpp
// Entire app in main.cpp
int main(int argc, char* argv[]) {
    Application app(argc, argv);
    Window window({.size = {400, 400}, .title = "Simple App"});
    window.setRootView(VStack {
        Text {.value = "Counter: " + std::to_string(count)},
        Button {.text = "Increment", .onClick = [&]() { count++; }}
    });
    return app.exec();
}
```

No separate UI files, no resource systems, no file management overhead. Just code.

## Direct Integration

Flux UI components are regular C++ objects. This means:

**No Boundaries**
- UI code and business logic share the same memory space
- Pass C++ data structures directly to UI components
- No marshaling or data conversion overhead
- Access any C++ library immediately

**No Boilerplate**
- No property registration systems
- No signal/slot declaration macros
- No type exposure mechanisms
- Just write C++ code

**Flexible Architecture**
```cpp
// Mix declarative and imperative naturally
auto text = Text{.fontSize = 48, .value = "Hello"};

if (darkMode) {
    text.color = Colors::White;
} else {
    text.color = Colors::Black;
}

window.setRootView(text);
```

Switch between declarative and imperative styles wherever it makes sense. No DSL limitations.

## Straightforward Debugging

Debug your UI the same way you debug any C++ code:

- Set breakpoints anywhere in your code
- Inspect UI objects as regular C++ objects
- Step through UI construction and updates
- Use standard C++ debugging tools
- No language boundary complications

## Transparent Performance

**Compiled Ahead of Time**
- Everything compiled to native code
- No runtime interpretation
- No JIT compilation overhead
- Predictable performance characteristics

**Direct Memory Control**
- Standard C++ memory management
- Use smart pointers, RAII, and other C++ patterns
- Profile with standard C++ profilers
- Optimize with familiar techniques

**Minimal Runtime**
- No additional runtime engines
- No interpreter overhead
- Lean deployment footprint

## Deployment Made Easy

**Single Binary Distribution**
- Compile your app to a single executable
- No separate resource files to manage
- No runtime modules to package
- Straightforward distribution process

**Small Footprint**
- Only link what you use
- No heavy runtime dependencies
- Efficient resource usage
- Fast startup times

## Learn Once, Use Everywhere

**One Documentation Source**
- Learn C++ concepts once
- Apply them to both UI and logic
- No separate documentation for UI language
- Consistent patterns throughout

**Familiar Patterns**
- If you know C++, you know Flux
- Standard C++ idioms and best practices apply
- No new programming paradigms to learn
- Immediate productivity

## When Flux Shines

Flux is particularly well-suited for:

**C++ Native Projects**
- Games and game engines
- Real-time systems
- High-performance applications
- Scientific computing tools
- Systems programming

**Embedded Systems**
- Resource-constrained environments
- Where runtime overhead matters
- Predictable performance requirements
- Minimal dependency footprint

**Developer Experience Priorities**
- Teams that value simplicity
- Projects with C++ expertise
- Codebases where debugging ease matters
- Applications requiring tight integration

**Rapid Prototyping**
- Quick experiments and demos
- Single-file applications
- Fast iteration cycles
- Minimal setup overhead

## The Flux Promise

Flux offers a clear value proposition:

✓ **Simplicity through unification** - One language, one toolchain, one way of thinking  
✓ **Modern C++ idioms** - Leverage the latest C++ features for clean, readable code  
✓ **Direct and transparent** - No hidden complexity, no magic, just C++  
✓ **Fast builds** - Standard compilation, no code generation overhead  
✓ **Easy debugging** - Use the tools you already know and trust  
✓ **Flexible architecture** - Write code the way that makes sense for your problem  

## Get Started

The simplest Flux app is just a few lines of C++:

```cpp
#include <Flux.hpp>
using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);
    Window window({.size = {400, 400}, .title = "My First Flux App"});
    window.setRootView(Text {.value = "Welcome to Flux!"});
    return app.exec();
}
```

Build it, run it, extend it. It's just C++.

---

## Philosophy

Flux is built on the belief that UI development shouldn't require learning new languages or dealing with complex build systems. If you know C++, you can build beautiful, performant user interfaces with Flux.

We choose simplicity over magic, directness over abstraction, and the power of modern C++ over framework-specific DSLs.

**Flux: Modern C++ UI, without compromise.**

