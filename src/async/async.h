#pragma once


#include <cstddef>
#include <cstdint>


namespace async 
{

using handle_t = std::uint64_t;

// Create a new processing context with bulk size N.
// Returns an opaque handle to be used in receive() and disconnect().
handle_t connect(std::size_t bulk_size);

// Feed a portion of input data into the context. Data may contain partial lines.
void receive(handle_t handle, const char* data, std::size_t size);

// Finalize the context, flushing any pending block and releasing resources.
void disconnect(handle_t handle);

} // namespace async

