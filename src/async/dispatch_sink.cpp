
#include "dispatch_sink.h"
#include "dispatcher.h"


namespace async 
{

// centralized call that will spread the info everywhere needed
void DispatchSink::onBulk(const bulkapp::Bulk& b) 
{
    Dispatcher::instance().dispatch(b);
}

} // namespace async

