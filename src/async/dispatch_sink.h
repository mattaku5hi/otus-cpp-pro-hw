#pragma once

#include <memory>

#include "model/ibulk_listener.h"

namespace async {

// A listener that forwards bulks to the global async dispatcher threads.
class DispatchSink : public bulkapp::IBulkListener {
public:
    void onBulk(const bulkapp::Bulk& b) override;
};

using DispatchSinkPtr = std::shared_ptr<DispatchSink>;

} // namespace async
