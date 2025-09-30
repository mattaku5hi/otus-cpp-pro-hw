#pragma once


#include <iterator>
#include <map>
#include <tuple>
#include <type_traits>
#include <utility>


template <typename T, T defaultValue>
class Matrix
{
public:
    using value_type = T;
    using coord_type = std::pair<int, int>;

    // Forward declarations
    // Proxy for the second indexing operator: matrix[x][y]
    class CellProxy;
    // Proxies representing matrix[x] indexing operator
    class RowProxy;
    class ConstRowProxy;

    // Iterator that yields std::tuple<x, y, v(value)>
    class Iterator 
    {
    public:
        // create nested type alias
        // it's implementation-defined for std::map container
        // type depends on T parameter so typename is used
        using underlying_iterator = typename std::map<coord_type, T>::const_iterator;
        // here we define the iterator category (std::iterator_traits) for STL 
        // it's iterator (not container) property
        // std::iterator_traits reads nested typedefs from the iterator type 
        // to define iterator category
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag; // for C++20
        using difference_type = std::ptrdiff_t;
        using value_type = std::tuple<int, int, T>;
        using reference = value_type;   // returning by value
        using pointer = void;           // not a pointer iterator

        Iterator() = default;
        explicit Iterator(underlying_iterator it) : m_it(it) {}

        reference operator*() const 
        {
            const auto& kv = *m_it;
            return std::make_tuple(kv.first.first, kv.first.second, kv.second);
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
    RowProxy operator[](int x) 
    { 
        return RowProxy(*this, x); 
    }
    // First-level indexing (const)
    ConstRowProxy operator[](int x) const 
    { 
        return ConstRowProxy(*this, x); 
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

    // Proxy representing matrix[x]
    class RowProxy 
    {
    public:
        RowProxy(Matrix& m, int x) : m_matrix(m), m_x(x) {}

        // Access cell proxy: matrix[x][y]
        class CellProxy 
        {
            public:
                CellProxy(Matrix& m, int x, int y) : m_matrix(m), m_x(x), m_y(y) {}
    
                // Read as value
                operator T() const 
                { 
                    return m_matrix.getValue(m_x, m_y); 
                }
    
                // Assign value (supports chained assignments)
                CellProxy& operator=(const T& v) 
                {
                    m_matrix.setValue(m_x, m_y, v);
                    return *this;
                }
    
                // Support chained assignment from another CellProxy
                CellProxy& operator=(const CellProxy& other) 
                {
                    T v = static_cast<T>(other);
                    m_matrix.setValue(m_x, m_y, v);
                    return *this;
                }

            private:
                Matrix& m_matrix;
                int m_x{};
                int m_y{};
        };

        CellProxy operator[](int y) 
        { 
            return CellProxy(m_matrix, m_x, y); 
        }

    private:
        Matrix& m_matrix;
        int m_x{};
    };
    
    // Proxy for const access: matrix[x][y] on const matrix returns value
    class ConstRowProxy 
    {
    public:
        ConstRowProxy(const Matrix& m, int x) : m_matrix(m), m_x(x) {}
        T operator[](int y) const 
        { 
            return m_matrix.getValue(m_x, y); 
        }
    private:
        const Matrix& m_matrix;
        int m_x{};
    };

private:
    friend class RowProxy;
    friend class ConstRowProxy;
    friend class CellProxy;

    static constexpr T kDefault = defaultValue;

    // Underlying sparse storage (keeps only non-default values)
    std::map<coord_type, T> m_storage;

    T getValue(int x, int y) const 
    {
        auto it = m_storage.find({x, y});
        return (it == m_storage.end()) ? kDefault : it->second;
    }

    void setValue(int x, int y, const T& v) 
    {
        if(v == kDefault) 
        {
            auto it = m_storage.find({x, y});
            if(it != m_storage.end())
            {
                m_storage.erase(it);
            }
        } 
        else 
        {
            m_storage[{x, y}] = v;
        }
    }

};
