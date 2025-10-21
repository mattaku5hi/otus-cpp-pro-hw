#pragma once

#include <memory>
#include <string>
#include <vector>

#include "options.h"

namespace bayan 
{

class Scanner 
{
public:
    // Returns groups of absolute file paths that are identical; only groups with size > 1 are returned.
    std::vector<std::vector<std::string>> findDuplicates(const Options& opts) const;
};

} // namespace bayan

