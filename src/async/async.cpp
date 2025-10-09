
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>

#include "aggregator.h"
#include "async.h"
#include "dispatch_sink.h"
#include "dispatcher.h"
#include "notifier.h"


namespace async 
{

struct Context 
{
    explicit Context(std::size_t n)
        : notifier(), aggregator(n, notifier) 
        {
        // Subscribe dispatcher sink: it will forward bulks to log and file threads
        auto sink = std::make_shared<DispatchSink>();
        notifier.subscribe(sink);
    }

    bulkapp::Notifier notifier;
    bulkapp::Aggregator aggregator;
    std::string buffer; // partial line buffer across receives
};

static std::mutex g_mutex;
static std::unordered_map<handle_t, std::shared_ptr<Context>> g_contexts;
static std::atomic<std::uint64_t> g_next_handle{1};


handle_t connect(std::size_t bulk_size) 
{
    // Start dispatcher threads on first use
    Dispatcher::instance().start();

    auto handle = g_next_handle.fetch_add(1, std::memory_order_relaxed);
    auto ctx = std::make_shared<Context>(bulk_size);
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        g_contexts.emplace(handle, std::move(ctx));
    }
    return handle;
}

void receive(handle_t handle, const char* data, std::size_t size) 
{
    if (data == nullptr || size == 0) return;

    std::shared_ptr<Context> ctx;
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        
        auto it = g_contexts.find(handle);
        if(it == g_contexts.end())
        {
            return; // unknown handle, ignore
        }
        ctx = it->second;
    }

    // Append and split by '\n'
    ctx->buffer.append(data, size);

    std::size_t pos = 0;
    while(true) 
    {
        auto newLinePos = ctx->buffer.find('\n', pos);
        if(newLinePos == std::string::npos)
        {
            // keep the remainder in buffer
            if(pos > 0) 
            {
                ctx->buffer.erase(0, pos);
            }
            break;
        }

        // Extract line without newline
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

void disconnect(handle_t handle) 
{
    std::shared_ptr<Context> ctx;
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        auto it = g_contexts.find(handle);
        if(it == g_contexts.end())
        {
            return; // unknown handle
        }
        
        ctx = it->second;
        g_contexts.erase(it);
    }

    // If there is a pending partial line, treat it as a complete command
    if(ctx->buffer.empty() == false)
    {
        ctx->aggregator.onLine(ctx->buffer);
        ctx->buffer.clear();
    }

    // Signal end-of-input for this context
    ctx->aggregator.onEof();
    // Context destroyed when shared_ptr goes out of scope
}

} // namespace async


