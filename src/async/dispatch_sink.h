#pragma once

#include <memory>

#include "model/ibulk_listener.h"

namespace async 
{

class IDispatcher;

// A listener that forwards bulks to the global async dispatcher threads.
class DispatchSink : public bulkapp::IBulkListener 
{
public:
    explicit DispatchSink(std::shared_ptr<IDispatcher> dispatcher)
        : m_dispatcher(std::move(dispatcher)) {}
    void onBulk(const bulkapp::Bulk& b) override;

private:
    std::shared_ptr<IDispatcher> m_dispatcher;
};

using DispatchSinkPtr = std::shared_ptr<DispatchSink>;

} // namespace async
