#pragma once


#include <memory>
#include <vector>

#include "ibulk_listener.h"


namespace bulkapp 
{

class Notifier
{
public:
    void subscribe(const IBulkListenerPtr& listener)
    {
        m_listeners.push_back(listener);
    }

    void publish(const Bulk& b)
    {
        for(auto& lis : m_listeners)
        {
            if(lis != nullptr)
            {
                lis->onBulk(b);
            }
        }
    }

private:
    std::vector<IBulkListenerPtr> m_listeners;
};

} // namespace bulkapp
