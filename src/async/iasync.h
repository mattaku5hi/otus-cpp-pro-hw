#pragma once


#include <cstddef>
#include <cstdint>


namespace async 
{

using handle_t = std::uint64_t;

class IAsyncEngine
{
public:
    virtual ~IAsyncEngine() = default;
    virtual handle_t connect(std::size_t bulk_size) = 0;
    virtual void receive(handle_t handle, const char* data, std::size_t size) = 0;
    virtual void disconnect(handle_t handle) = 0;
};

} // namespace async
