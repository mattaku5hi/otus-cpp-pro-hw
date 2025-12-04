#pragma once


#include <atomic>
#include <string>

#include "model/ibulk_listener.h"


namespace bulkapp 
{

class FileSink : public IBulkListener
{
public:
    explicit FileSink(std::string directory = "", std::string threadId = "")
        : m_dir(std::move(directory)), m_threadId(std::move(threadId)) {}
    void onBulk(const Bulk& b) override;

private:
    std::string m_dir; // optional directory prefix, may be empty, must end with '/' if non-empty
    std::string m_threadId; // optional id to make filenames unique per file thread
    std::atomic<unsigned long long> m_seq{0}; // monotonic sequence per sink instance
};

} // namespace bulkapp
