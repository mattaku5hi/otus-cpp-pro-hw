
#include "dispatch_sink.h"
#include "idispatcher.h"


namespace async 
{

// centralized call that will spread the info everywhere needed
void DispatchSink::onBulk(const bulkapp::Bulk& b) 
{
    m_dispatcher->dispatch(b);
}

} // namespace async

