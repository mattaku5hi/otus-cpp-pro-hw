
#include <chrono>

#include "aggregator.h"
#include "bulk.h"
#include "notifier.h"


namespace bulkapp 
{

void Aggregator::onLine(const std::string& line)
{
    if(line == "{")
    {
        if(m_depth == 0)
        {
            // received new-block symbol, let's print a previous block
            this->flush();
        }
        ++m_depth;
        return;
    }

    if(line == "}")
    {
        if(m_depth > 0)
        {
            --m_depth;
            // received closing brace, let's print a current block
            if(m_depth == 0)
            {
                this->flush();
            }
        }
        // else: stray closing brace outside dynamic block - ignore
        return;
    }

    // regular command
    if(m_buffer.empty() == true)
    {
        // it's the first cmd in block, let's save a timestamp
        m_firstTs = std::time(nullptr);
    }
    m_buffer.push_back(line);

    if(m_depth == 0 && m_buffer.size() == m_N)
    {
        this->flush();
    }
}

// met EOF symbol, let's print a current block
void Aggregator::onEof()
{
    if(m_depth == 0)
    {
        this->flush();
    }
    // if m_depth > 0: ignore unfinished dynamic block as per spec
}

void Aggregator::flush()
{
    if(m_buffer.empty() == true)
    {
        return;
    }

    Bulk bulk;
    bulk.timestamp = m_firstTs.value_or(std::time(nullptr));
    bulk.commands = std::move(m_buffer);

    m_notifier.publish(bulk);

    m_buffer.clear();
    m_buffer.shrink_to_fit();
    m_firstTs.reset();
}


} // namespace bulkapp

