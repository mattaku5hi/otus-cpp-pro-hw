#pragma once


#include <optional>
#include <string>


namespace csv_utils
{

std::string_view getField(const std::string_view line, size_t target_idx);
std::optional<std::string_view> getKey(const std::string_view line)  noexcept;
std::optional<double> getPrice(const std::string_view line) noexcept;

} // namespace csv_utils
