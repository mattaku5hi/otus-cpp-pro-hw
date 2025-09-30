#pragma once


#include <iterator>
#include <map>
#include <tuple>
#include <type_traits>
#include <utility>
#include <array>


template <typename T, T defaultValue, std::size_t Dim = 2>
class Matrix
{
public:
    using value_type = T;
    static constexpr std::size_t dimension = Dim;
    using index_type = std::array<int, Dim>;
    using storage_type = std::map<index_type, T>;

    // Forward declarations for proxy-based indexing
    class CellProxy;
    template <std::size_t Depth>
    class IndexProxy;
    template <std::size_t Depth>
    class ConstIndexProxy;

    // Iterator that yields std::tuple<x, y, v(value)>
    class Iterator 
    {
    public:
        // create nested type alias
        // it's implementation-defined for std::map container
        // type depends on T parameter so typename is used
        using underlying_iterator = typename storage_type::const_iterator;
        // here we define the iterator category (std::iterator_traits) for STL 
        // it's iterator (not container) property
        // std::iterator_traits reads nested typedefs from the iterator type 
        // to define iterator category
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag; // for C++20
        using difference_type = std::ptrdiff_t;
        using iter_value_type = std::conditional_t<
            (Dim == 2),
            std::tuple<int, int, T>,
            std::pair<index_type, T>
        >;
        using value_type = iter_value_type;
        using reference = iter_value_type;   // returning by value
        using pointer = void;           // not a pointer iterator

        Iterator() = default;
        explicit Iterator(underlying_iterator it) : m_it(it) {}

        reference operator*() const 
        {
            const auto& kv = *m_it; // kv: pair<index_type, T>
            if constexpr (Dim == 2)
            {
                return std::make_tuple(kv.first[0], kv.first[1], kv.second);
            }
            else
            {
                return std::make_pair(kv.first, kv.second);
            }
        }

        Iterator& operator++() 
        {
            ++m_it;
            return *this;
        }

        Iterator operator++(int) 
        {
            Iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) 
        { 
            return a.m_it == b.m_it; 
        }
        
        friend bool operator!=(const Iterator& a, const Iterator& b) 
        { 
            return !(a == b);
            // bro remember about endless recursion:
            // return (a != b);
        }

    private:
        underlying_iterator m_it{};
    };

    // Size of occupied cells
    std::size_t size() const noexcept 
    { 
        return m_storage.size(); 
    }

    // First-level indexing (non-const)
    IndexProxy<1> operator[](int first)
    {
        index_type idx{};
        idx.fill(0);
        idx[0] = first;
        return IndexProxy<1>(*this, idx);
    }
    // First-level indexing (const)
    ConstIndexProxy<1> operator[](int first) const
    {
        index_type idx{};
        idx.fill(0);
        idx[0] = first;
        return ConstIndexProxy<1>(*this, idx);
    }

    // Iteration over occupied cells
    Iterator begin() const 
    { 
        return Iterator(m_storage.cbegin()); 
    }
    Iterator end() const 
    { 
        return Iterator(m_storage.cend()); 
    }

    // Non-const proxy chain for operator[] ... operator[] (Dim times)
    template <std::size_t Depth>
    class IndexProxy
    {
    public:
        IndexProxy(Matrix& m, index_type idx) : m_matrix(m), m_idx(idx) {}

        auto operator[](int coord)
        {
            static_assert(Depth < Dim, "IndexProxy depth overflow");
            m_idx[Depth] = coord; // fill next coordinate (0-based)
            if constexpr(Depth + 1 < Dim)
            {
                return IndexProxy<Depth + 1>(m_matrix, m_idx);
            }
            else
            {
                return CellProxy(m_matrix, m_idx);
            }
        }

    private:
        Matrix& m_matrix;
        index_type m_idx{};
    };

    // Const proxy chain for operator[] ... operator[] (Dim times)
    template <std::size_t Depth>
    class ConstIndexProxy
    {
    public:
        ConstIndexProxy(const Matrix& m, index_type idx) : m_matrix(m), m_idx(idx) {}

        auto operator[](int coord) const
        {
            static_assert(Depth < Dim, "ConstIndexProxy depth overflow");
            m_idx[Depth] = coord; // fill next coordinate (0-based)
            if constexpr(Depth + 1 < Dim)
            {
                return ConstIndexProxy<Depth + 1>(m_matrix, m_idx);
            }
            else
            {
                return m_matrix.getValue(m_idx);
            }
        }

    private:
        const Matrix& m_matrix;
        index_type m_idx{};
    };

    // Final cell proxy (non-const) enabling read/assign/chained-assign
    class CellProxy
    {
    public:
        CellProxy(Matrix& m, const index_type& idx) : m_matrix(m), m_idx(idx) {}

        operator T() const 
        { 
            return m_matrix.getValue(m_idx); 
        }

        CellProxy& operator=(const T& v)
        {
            m_matrix.setValue(m_idx, v);
            return *this;
        }

        CellProxy& operator=(const CellProxy& other)
        {
            T v = static_cast<T>(other);
            m_matrix.setValue(m_idx, v);
            return *this;
        }

    private:
        Matrix& m_matrix;
        index_type m_idx{};
    };

private:
    template <std::size_t>
    friend class IndexProxy;
    template <std::size_t>
    friend class ConstIndexProxy;
    friend class CellProxy;

    static constexpr T kDefault = defaultValue;

    // Underlying sparse storage (keeps only non-default values)
    storage_type m_storage;

    T getValue(const index_type& idx) const 
    {
        auto it = m_storage.find(idx);
        return (it == m_storage.end()) ? kDefault : it->second;
    }

    void setValue(const index_type& idx, const T& v) 
    {
        if(v == kDefault) 
        {
            auto it = m_storage.find(idx);
            if(it != m_storage.end())
            {
                m_storage.erase(it);
            }
        } 
        else 
        {
            m_storage[idx] = v;
        }
    }

};

