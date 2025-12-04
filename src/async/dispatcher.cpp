
#include <cassert>
#include <utility>

#include "dispatcher.h"


namespace async 
{

Dispatcher::~Dispatcher() 
{
    stop();
}

void Dispatcher::start() 
{
    bool expected = false;
    if(m_started.compare_exchange_strong(expected, true) == false) 
    {
        return; // already started
    }

    if(m_fileSinksInited == false) 
    {
        // Initialize file sinks with unique IDs to ensure unique filenames
        m_fileSink1 = std::make_unique<::bulkapp::FileSink>("", "f1");
        m_fileSink2 = std::make_unique<::bulkapp::FileSink>("", "f2");
        m_fileSinksInited = true;
    }

    // create all the necessary threads
    m_logThread = std::thread([this] { 
            runLog(); 
        });
    m_fileThread1 = std::thread([this] { 
            runFile(m_fileSink1.get()); 
        });
    m_fileThread2 = std::thread([this] { 
            runFile(m_fileSink2.get()); 
        });
}

void Dispatcher::stop() 
{
    if(m_started.exchange(false) == false) 
    {
        return; // not started
    }

    // Signal queues to stop and join threads
    m_logQueue.stop();
    m_fileQueue.stop();

    if(m_logThread.joinable())
    {
        m_logThread.join();
    }
    if(m_fileThread1.joinable())
    {
        m_fileThread1.join();
    }
    if(m_fileThread2.joinable())
    {
        m_fileThread2.join();
    }
}

void Dispatcher::dispatch(const ::bulkapp::Bulk& b) 
{
    // push copies to queues
    m_logQueue.push(b);
    m_fileQueue.push(b);
}

void Dispatcher::runLog() 
{
    ::bulkapp::Bulk b;
    while(true) 
    {
        if(m_logQueue.waitPop(b) == false) 
        {
            break;
        }
        m_consoleSink.onBulk(b);
    }
}

void Dispatcher::runFile(::bulkapp::FileSink* sink) 
{
    assert(sink != nullptr);
    ::bulkapp::Bulk bulk;
    while(true) 
    {
        if(m_fileQueue.waitPop(bulk) == false) 
        {
            break;
        }
        sink->onBulk(bulk);
    }
}

} // namespace async


