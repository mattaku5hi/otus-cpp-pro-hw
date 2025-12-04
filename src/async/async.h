#pragma once


#include <cstddef>
#include <cstdint>
#include <memory>

#include "iasync.h"

namespace async 
{

class IDispatcher; // fwd

class AsyncEngine : public IAsyncEngine
{
public:
    AsyncEngine(std::shared_ptr<IDispatcher> dispatcher);
    handle_t connect(std::size_t bulk_size) override;
    void receive(handle_t handle, const char* data, std::size_t size) override;
    void disconnect(handle_t handle) override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};


std::shared_ptr<IAsyncEngine> createEngine(std::shared_ptr<IDispatcher> dispatcher);


} // namespace async

