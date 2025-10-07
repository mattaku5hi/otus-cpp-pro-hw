#pragma once


#include <string>

#include "ibulk_listener.h"


namespace bulkapp 
{

class FileSink : public IBulkListener
{
public:
    explicit FileSink(std::string directory = "") : m_dir(std::move(directory)) {}
    void onBulk(const Bulk& b) override;

private:
    std::string m_dir; // optional directory prefix, may be empty, must end with '/' if non-empty
};

} // namespace bulkapp

