
#pragma once
#include <iostream>
#include <list>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

/**
 * @file print_ip.h
 * @brief SFINAE-based overload set for printing a "conditional IP address".
 *
 * Supported inputs:
 *  - Any integral type (except bool): prints all bytes MSB->LSB separated by '.'
 *  - std::string: prints as-is
 *  - std::vector<T>, std::list<T>: prints elements separated by '.'
 *  - std::tuple<T, T, ... T>: prints elements separated by '.' (all types must be identical)
 */

namespace hw3 {


// encapsulte by means of namespace :)
namespace custom {

template <typename T>
struct is_std_vector : std::false_type {};

template <typename... Args>
struct is_std_vector<std::vector<Args...>> : std::true_type {};

template <typename T>
struct is_std_list : std::false_type {};

template <typename... Args>
struct is_std_list<std::list<Args...>> : std::true_type {};

template <typename T>
using is_std_string = std::is_same<std::decay_t<T>, std::string>;

// Print tuple recursively
// Recursion termination
// template <std::size_t I = 0, typename... Ts>
// std::enable_if_t<I == sizeof...(Ts), void>
// tuple_print(const std::tuple<Ts...>&) {}
// // Recursive step
// template <std::size_t I = 0, typename... Ts>
// std::enable_if_t<I < sizeof...(Ts), void>
// tuple_print(const std::tuple<Ts...>& t) {
//     if constexpr(I > 0) 
//     {
//         std::cout << '.';
//     }
//     std::cout << std::get<I>(t);
//     tuple_print<I + 1>(t);
// }

// or like that (C++17 approach): all the recursive operation in one function
template <std::size_t I = 0, typename... Ts>
void tuple_print(const std::tuple<Ts...>& t) 
{
    if constexpr(I < sizeof...(Ts))
    {
        if constexpr(I > 0) 
        {
            std::cout << '.';
        }
        std::cout << std::get<I>(t);
        tuple_print<I + 1>(t);
    }
}


}  // namespace custom

/**
 * @brief Print all bytes of an integral value from MSB to LSB separated by '.'.
 */
template <typename T, std::enable_if_t<std::is_integral_v<std::remove_reference_t<T>> &&
                            !std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, bool>, int> = 0>
void print_ip(T value)
{
    using U = std::make_unsigned_t<std::remove_cv_t<std::remove_reference_t<T>>>;
    U u = static_cast<U>(value);
    constexpr std::size_t N = sizeof(U);
    for(std::size_t i = 0; i < N; ++i)
    {
        std::size_t shift = 8 * (N - 1 - i);
        auto byte = static_cast<unsigned int>((u >> shift) & static_cast<U>(0xFF));
        if(i > 0)
        {
            std::cout << '.';
        }
        std::cout << byte;
    }
    std::cout << std::endl;
}

/**
 * @brief Print std::string as-is.
 */
template <typename T, std::enable_if_t<custom::is_std_string<T>::value, int> = 0>
void print_ip(const T& s)
{
    std::cout << s << std::endl;
}

/**
 * @brief Print elements of std::vector or std::list separated by '.'.
 */
template <typename T,
/**
 * @brief Print elements of std::vector or std::list separated by '.'.
 *
 * Iterates over container elements and prints them in order with '.' as a
 * delimiter, then prints a trailing newline.
 *
 * @tparam T Either std::vector<...> or std::list<...> (cv/ref qualified forms allowed).
 * @param container Container to print.
 */
          std::enable_if_t<custom::is_std_vector<std::remove_cv_t<std::remove_reference_t<T>>>::value ||
                               custom::is_std_list<std::remove_cv_t<std::remove_reference_t<T>>>::value,
                           int> = 0>
void print_ip(const T& container)
{
    bool first = true;
    for(const auto& elem : container)
    {
        if(first == false)
        {
            std::cout << '.';
        }
        std::cout << elem;
        first = false;
    }
    std::cout << std::endl;
}

// for clearer troubleshooting
template <typename... Ts,
          std::enable_if_t<(sizeof...(Ts) > 0) &&
                           !(std::is_same_v<Ts, std::tuple_element_t<0, std::tuple<Ts...>>> && ...), int> = 0>
/**
 * @brief Deleted overload for tuples with non-uniform element types.
 *
 * This function is intentionally deleted to produce a clear compile-time
 * diagnostic when a std::tuple with differing element types is passed.
 *
 * @tparam Ts Tuple element types; participates only when not all Ts are equal
 *            to the first element type.
 */
void print_ip(const std::tuple<Ts...>&) = delete;

/**
 * @brief Print elements of std::tuple<T, T, ... T> separated by '.'.
 *        SFINAE ensures all types are identical; otherwise there is no viable overload.
 */
template <typename... Ts,
          std::enable_if_t<(sizeof...(Ts) > 0) &&
            // compute a pack of type traits whaaaat
            std::conjunction_v<std::is_same<Ts, std::tuple_element_t<0, std::tuple<Ts...>>>...>,
                           int> = 0>
/**
 * @brief Print elements of a uniform std::tuple separated by '.'.
 *
 * Enabled only when all tuple element types are identical. Elements are written
 * in order and separated by '.'. A trailing newline is printed at the end.
 *
 * @tparam Ts Tuple element types (all must be the same type).
 * @param t Tuple to print.
 */
void print_ip(const std::tuple<Ts...>& t) 
{
    custom::tuple_print(t);
    std::cout << std::endl;
}


} // namespace hw3
