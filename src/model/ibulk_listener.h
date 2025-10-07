#pragma once


#include <memory>

#include "model/bulk.h"


namespace bulkapp 
{

class IBulkListener
{
public:
    virtual ~IBulkListener() = default;
    virtual void onBulk(const Bulk& b) = 0;
};

using IBulkListenerPtr = std::shared_ptr<IBulkListener>;


} // namespace bulkapp

