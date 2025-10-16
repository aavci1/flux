#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <vector>
#include <iostream>
#include <string>

namespace flux {

// Forward declarations
class View;
struct LayoutNode;

// Debug function to print the layout tree structure
// Note: Implementation is provided at the end of this file if View.hpp is complete
void printLayoutTree(const LayoutNode& node, int depth = 0, const std::string& prefix = "");

} // namespace flux

// Implementation section - only compile if View.hpp has been fully processed
#ifdef FLUX_VIEW_HPP_COMPLETE

namespace flux {

inline void printLayoutTree(const LayoutNode& node, int depth, const std::string& prefix) {
    // Create indentation based on depth
    std::string indent(depth * 2, ' ');

    // Get the type name of the view (demangled)
    std::string typeName = node.view.getTypeName();

    // Print current node info with the actual type name
    std::cout << indent << prefix << typeName << " "
              << "["
              << "x:" << node.bounds.x << ", "
              << "y:" << node.bounds.y << ", "
              << "w:" << node.bounds.width << ", "
              << "h:" << node.bounds.height
              << "]";

    // Print number of children
    if (!node.children.empty()) {
        std::cout << " children:" << node.children.size();
    }

    std::cout << std::endl;

    // Recursively print children
    for (size_t i = 0; i < node.children.size(); ++i) {
        bool isLast = (i == node.children.size() - 1);
        std::string childPrefix = isLast ? "└─ " : "├─ ";
        printLayoutTree(node.children[i], depth + 1, childPrefix);
    }
}

} // namespace flux

#endif // FLUX_VIEW_HPP_COMPLETE
