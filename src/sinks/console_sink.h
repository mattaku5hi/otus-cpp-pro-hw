#pragma once


#include <iostream>
#include <memory>
#include <string>

#include "bulk.h"
#include "ibulk_listener.h"


namespace bulkapp 
{

class ConsoleSink : public IBulkListener
{
public:
    void onBulk(const Bulk& b) override;
};


} // namespace bulkapp

