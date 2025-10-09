#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "model/bulk.h"
#include "sinks/console_sink.h"
#include "sinks/file_sink.h"
#include "thread_safe_queue.h"


namespace async 
{

class Dispatcher 
{
public:
    static Dispatcher& instance();

    void start();
    void stop();

    void dispatch(const bulkapp::Bulk& b);

private:
    Dispatcher() = default;
    ~Dispatcher();

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    void runLog();
    void runFile(bulkapp::FileSink* sink);

    std::atomic<bool> m_started{false};

    ThreadSafeQueue<bulkapp::Bulk> m_logQueue;
    ThreadSafeQueue<bulkapp::Bulk> m_fileQueue;

    std::thread m_logThread;
    std::thread m_fileThread1;
    std::thread m_fileThread2;

    bulkapp::ConsoleSink m_consoleSink;
    std::unique_ptr<bulkapp::FileSink> m_fileSink1; // id "f1"
    std::unique_ptr<bulkapp::FileSink> m_fileSink2; // id "f2"

    bool m_fileSinksInited{false};
};

} // namespace async

