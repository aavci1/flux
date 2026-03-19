#include <Flux/Core/View.hpp>
#include <string>
#include <cstdlib>

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#elif defined(_MSC_VER)
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")
#endif

namespace flux {

std::string demangleTypeName(const char* mangledName) {
#if defined(__GNUC__) || defined(__clang__)
    int status = 0;
    char* demangled = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);
    if (status == 0 && demangled) {
        std::string result(demangled);
        std::free(demangled);
        return result;
    }
#elif defined(_MSC_VER)
    char buf[1024];
    if (UnDecorateSymbolName(mangledName, buf, sizeof(buf), UNDNAME_COMPLETE)) {
        return std::string(buf);
    }
#endif
    return std::string(mangledName);
}

} // namespace flux
