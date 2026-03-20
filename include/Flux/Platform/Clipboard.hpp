#pragma once

#include <string>

namespace flux {

class Clipboard {
public:
    virtual ~Clipboard() = default;
    virtual void setText(const std::string& text) = 0;
    virtual std::string getText() = 0;
    virtual bool hasText() = 0;
};

} // namespace flux
