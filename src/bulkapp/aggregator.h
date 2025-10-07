#pragma once


#include <ctime>
#include <optional>
#include <string>
#include <vector>

#include "notifier.h"


namespace bulkapp 
{

class Aggregator
{
public:
    explicit Aggregator(std::size_t n, Notifier& notifier)
        : m_N(n), m_notifier(notifier) {}

    void onLine(const std::string& line);
    void onEof();

private:
    void flush();

    const std::size_t m_N;
    Notifier& m_notifier;

    std::size_t m_depth{0};
    std::vector<std::string> m_buffer;
    std::optional<std::time_t> m_firstTs{};
};

} // namespace bulkapp

