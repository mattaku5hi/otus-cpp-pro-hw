
#include <sstream>

#include "id_generator.h"


/// @brief Generate new identifier
/// @param prefix    textual prefix (e.g., "line", "rect")
/// @return          identifier in format "<prefix>-<N>"
std::string IdGenerator::Generate(const std::string& prefix)
{
    auto it = m_counters.find(prefix);
    if(it == m_counters.end())
    {
        m_counters[prefix] = 1;
    }
    else
    {
        it->second += 1;
    }

    std::size_t value = m_counters[prefix];

    std::ostringstream os;
    os << prefix << '-' << value;
    return os.str();
}

