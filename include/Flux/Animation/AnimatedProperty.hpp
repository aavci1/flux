#pragma once

#include <Flux/Core/Property.hpp>
#include <Flux/Animation/Animatable.hpp>

namespace flux {

// AnimatedProperty<T> marks a Property as participating in the implicit
// animation system.  It is a constrained alias for Property<T> — the
// Animatable concept guarantees that a lerp() overload exists for T,
// so the reconciliation engine can interpolate between old and new values.
//
// Usage in view structs:
//
//   AnimatedProperty<float>  opacity = 1.0f;
//   AnimatedProperty<Color>  backgroundColor = Colors::transparent;
//
// Non-animatable properties (bool, string, …) keep using plain Property<T>.
template<Animatable T>
using AnimatedProperty = Property<T>;

} // namespace flux
