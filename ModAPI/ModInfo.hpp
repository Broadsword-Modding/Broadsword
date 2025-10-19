#pragma once

#include <string>

namespace Broadsword {

/**
 * Metadata about a mod
 */
struct ModInfo {
    std::string Name;
    std::string Version;
    std::string Author;
    std::string Description;
};

} // namespace Broadsword
