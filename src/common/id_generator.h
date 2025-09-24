#pragma once


#include <string>
#include <unordered_map>


/// @brief Identifier generator with prefixes (thread-unsafe, for simplicity)
class IdGenerator
{
public:
    /// @brief Generate new identifier
    /// @param prefix    textual prefix (e.g., "line", "rect")
    /// @return          identifier in format "<prefix>-<N>"
    std::string Generate(const std::string &prefix);

private:
    std::unordered_map<std::string, std::size_t> m_counters;
};

