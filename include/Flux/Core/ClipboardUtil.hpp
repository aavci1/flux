#pragma once

#include <string>

namespace flux {

void setClipboardText(const std::string& text);
bool hasClipboardText();
std::string getClipboardText();

} // namespace flux
