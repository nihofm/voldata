#include "dict.h"

#ifdef __GNUG__ // gnu C++ compiler
#include <cxxabi.h>
#include <stdlib.h>
#include <memory>
std::string demangle(const char* mangled) {
    int status;
    std::unique_ptr<char[], void (*)(void*)> result(abi::__cxa_demangle(mangled, 0, 0, &status), std::free);
    return result.get() ? std::string(result.get()) : "error occurred";
}
#else
std::string demangle(const char* name) { return name; }
#endif // _GNUG_

namespace voldata {

std::string Dictionary::to_string(const std::string& indent) const {
    std::stringstream out;
    out << indent << "size: " << map.size() << std::endl;
    out << indent << "content: ";
    for (auto [key, value] : map)
        out << key << " (" << demangle(value.type().name()) << "), ";
    return out.str();
}

}
