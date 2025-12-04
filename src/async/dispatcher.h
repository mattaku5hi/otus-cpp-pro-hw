#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "async/idispatcher.h"
#include "async/thread_safe_queue.h"
#include "sinks/console_sink.h"
#include "sinks/file_sink.h"


namespace async 
{


class Dispatcher : public IDispatcher
{
public:
    Dispatcher() = default;
    ~Dispatcher() override;

    void start() override;
    void stop() override;

    void dispatch(const bulkapp::Bulk& b) override;

private:
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    void runLog();
    void runFile(::bulkapp::FileSink* sink);

    std::atomic<bool> m_started{false};

    ThreadSafeQueue<bulkapp::Bulk> m_logQueue;
    ThreadSafeQueue<bulkapp::Bulk> m_fileQueue;

    std::thread m_logThread;
    std::thread m_fileThread1;
    std::thread m_fileThread2;

    ::bulkapp::ConsoleSink m_consoleSink;
    std::unique_ptr<::bulkapp::FileSink> m_fileSink1; // id "f1"
    std::unique_ptr<::bulkapp::FileSink> m_fileSink2; // id "f2"

    bool m_fileSinksInited{false};
};

} // namespace async


