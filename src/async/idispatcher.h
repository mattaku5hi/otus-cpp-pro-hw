#pragma once

namespace bulkapp 
{ 
    class Bulk; 
} // fwd in global namespace

namespace async 
{

class IDispatcher
{
public:
    virtual ~IDispatcher() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void dispatch(const bulkapp::Bulk& b) = 0;
};

} // namespace async
