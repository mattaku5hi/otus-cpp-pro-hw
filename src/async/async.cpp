
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "aggregator.h"
#include "async.h"
#include "dispatch_sink.h"
#include "idispatcher.h"
#include "notifier.h"


namespace async 
{

struct Context 
{
    explicit Context(std::size_t n, std::shared_ptr<IDispatcher> dispatcher)
        : notifier(), aggregator(n, notifier) 
    {
        // Subscribe dispatcher sink: it will forward bulks to log and file threads
        auto sink = std::make_shared<DispatchSink>(std::move(dispatcher));
        notifier.subscribe(sink);
    }

    bulkapp::Notifier notifier;
    bulkapp::Aggregator aggregator;
    std::string buffer; // partial line buffer across receives
};
struct AsyncEngine::Impl
{
    std::atomic<std::uint64_t> nextHandle{1};
    std::mutex mutex;
    std::unordered_map<handle_t, std::shared_ptr<Context>> contexts;
    std::shared_ptr<IDispatcher> dispatcher;
};

AsyncEngine::AsyncEngine(std::shared_ptr<IDispatcher> dispatcher)
    : m_impl(std::make_unique<Impl>())
{
    m_impl->dispatcher = std::move(dispatcher);
}

handle_t AsyncEngine::connect(std::size_t bulk_size)
{
    m_impl->dispatcher->start();

    auto handle = m_impl->nextHandle.fetch_add(1, std::memory_order_relaxed);
    auto ctx = std::make_shared<Context>(bulk_size, m_impl->dispatcher);
    {
        std::lock_guard<std::mutex> lk(m_impl->mutex);
        m_impl->contexts.emplace(handle, std::move(ctx));
    }
    return handle;
}

void AsyncEngine::receive(handle_t handle, const char* data, std::size_t size)
{
    if(data == nullptr || size == 0)
    {
        return;
    }

    std::shared_ptr<Context> ctx;
    {
        std::lock_guard<std::mutex> lk(m_impl->mutex);
        auto it = m_impl->contexts.find(handle);
        if(it == m_impl->contexts.end())
        {
            return;
        }
        ctx = it->second;
    }

    ctx->buffer.append(data, size);

    std::size_t pos = 0;
    while(true)
    {
        auto newLinePos = ctx->buffer.find('\n', pos);
        if(newLinePos == std::string::npos)
        {
            if(pos > 0)
            {
                ctx->buffer.erase(0, pos);
            }
            break;
        }

        std::string line = ctx->buffer.substr(pos, newLinePos - pos);
        ctx->aggregator.onLine(line);
        pos = newLinePos + 1;
        if(pos >= ctx->buffer.size())
        {
            ctx->buffer.clear();
            break;
        }
    }
}

void AsyncEngine::disconnect(handle_t handle)
{
    std::shared_ptr<Context> ctx;
    {
        std::lock_guard<std::mutex> lk(m_impl->mutex);
        auto it = m_impl->contexts.find(handle);
        if(it == m_impl->contexts.end())
        {
            return;
        }
        ctx = it->second;
        m_impl->contexts.erase(it);
    }

    if(ctx->buffer.empty() == false)
    {
        ctx->aggregator.onLine(ctx->buffer);
        ctx->buffer.clear();
    }
    ctx->aggregator.onEof();
}

std::shared_ptr<IAsyncEngine> createEngine(std::shared_ptr<IDispatcher> dispatcher)
{
    return std::make_shared<AsyncEngine>(std::move(dispatcher));
}


} // namespace async



