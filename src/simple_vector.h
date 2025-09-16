#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>


namespace hw3 {

// A simple fixed-capacity vector-like container using Alloc for storage.
template <class T, class Alloc = std::allocator<T>>
class SimpleVector 
{
public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    class iterator 
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator() = default;
        explicit iterator(pointer p) : m_ptr(p) {}

        reference operator*() const 
        { 
            return *m_ptr; 
        }
        pointer operator->() const 
        { 
            return m_ptr; 
        }

        iterator& operator++() 
        { 
            ++m_ptr; 
            return *this; 
        }
        iterator operator++(int) 
        { 
            iterator tmp(*this); 
            ++(*this); 
            return tmp; 
        }
        iterator& operator--() 
        { 
            --m_ptr; 
            return *this; 
        }
        iterator operator--(int) 
        { 
            iterator tmp(*this); 
            --(*this); 
            return tmp; 
        }

        friend bool operator==(const iterator& a, const iterator& b) 
        { 
            return a.m_ptr == b.m_ptr; 
        }
        friend bool operator!=(const iterator& a, const iterator& b) 
        { 
            return !(a == b); 
        }

        // Minimal random access ops
        iterator& operator+=(difference_type n) 
        { 
            m_ptr += n; 
            return *this; 
        }
        iterator& operator-=(difference_type n) 
        { 
            m_ptr -= n; 
            return *this; 
        }
        friend iterator operator+(iterator it, difference_type n) 
        { 
            it += n; 
            return it; 
        }
        friend iterator operator+(difference_type n, iterator it) 
        { 
            it += n; 
            return it; 
        }
        friend iterator operator-(iterator it, difference_type n) 
        { 
            it -= n; 
            return it; 
        }
        friend difference_type operator-(const iterator& a, const iterator& b) 
        { 
            return a.m_ptr - b.m_ptr; 
        }
        reference operator[](difference_type n) const 
        { 
            return *(m_ptr + n); 
        }
        friend bool operator<(const iterator& a, const iterator& b) 
        { 
            return a.m_ptr < b.m_ptr; 
        }
        friend bool operator>(const iterator& a, const iterator& b) 
        { 
            return b < a; 
        }
        friend bool operator<=(const iterator& a, const iterator& b) 
        { 
            return !(b < a); 
        }
        friend bool operator>=(const iterator& a, const iterator& b) 
        { 
            return !(a < b); 
        }

    private:
        pointer m_ptr{nullptr};
    };

    class const_iterator 
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = const T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator() = default;
        explicit const_iterator(pointer p) : m_ptr(p) 
        {}
        const_iterator(const iterator& it) : m_ptr(it.operator->()) 
        {}

        reference operator*() const 
        { 
            return *m_ptr; 
        }
        pointer operator->() const 
        { 
            return m_ptr; 
        }

        const_iterator& operator++() 
        { 
            ++m_ptr; 
            return *this; 
        }
        const_iterator operator++(int) 
        {
            const_iterator tmp(*this); 
            ++(*this); 
            return tmp; 
        }
        const_iterator& operator--() 
        { 
            --m_ptr; 
            return *this; 
        }
        const_iterator operator--(int)
        { 
            const_iterator tmp(*this); 
            --(*this); 
            return tmp; 
        }

        friend bool operator==(const const_iterator& a, const const_iterator& b) 
        { 
            return a.m_ptr == b.m_ptr; 
        }
        friend bool operator!=(const const_iterator& a, const const_iterator& b) 
        { 
            // do not use operator != to avoid endless recursion
            return !(a == b); 
        }

        // Minimal random access ops
        const_iterator& operator+=(difference_type n) 
        { 
            m_ptr += n; 
            return *this; 
        }
        const_iterator& operator-=(difference_type n) 
        { 
            m_ptr -= n; 
            return *this; 
        }
        friend const_iterator operator+(const const_iterator& it, difference_type n) 
        { 
            const_iterator r(it); 
            r += n; 
            return r; 
        }
        friend const_iterator operator+(difference_type n, const const_iterator& it) 
        { 
            return it + n; 
        }
        friend const_iterator operator-(const const_iterator& it, difference_type n) 
        { 
            const_iterator r(it); 
            r -= n; 
            return r; 
        }
        friend difference_type operator-(const const_iterator& a, const const_iterator& b) 
        { 
            return a.m_ptr - b.m_ptr; 
        }
        reference operator[](difference_type n) const 
        { 
            return *(m_ptr + n); 
        }
        friend bool operator<(const const_iterator& a, const const_iterator& b) 
        { 
            return a.m_ptr < b.m_ptr; 
        }
        friend bool operator>(const const_iterator& a, const const_iterator& b) 
        { 
            return b < a; 
        }
        friend bool operator<=(const const_iterator& a, const const_iterator& b) 
        { 
            return !(b < a); 
        }
        friend bool operator>=(const const_iterator& a, const const_iterator& b) 
        { 
            return !(a < b); 
        }

    private:
        pointer m_ptr{nullptr};
    };

    explicit SimpleVector(size_type capacity = 0, const Alloc& alloc = Alloc())
        : m_alloc(alloc), m_capacity(capacity) 
    {
        if(m_capacity > 0) 
        { 
            m_data = std::allocator_traits<Alloc>::allocate(m_alloc, m_capacity); 
        }
    }

    // Non-copyable for simplicity
    SimpleVector(const SimpleVector&) = delete;
    SimpleVector& operator=(const SimpleVector&) = delete;

    // Movable
    SimpleVector(SimpleVector&& other) noexcept
        : m_alloc(std::move(other.m_alloc)), m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity)
    {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }
    SimpleVector& operator=(SimpleVector&& other) noexcept
    {
        if(this != &other) 
        {
            clear();
            if(m_data) 
            { 
                std::allocator_traits<Alloc>::deallocate(m_alloc, m_data, m_capacity); 
            }
            m_alloc = std::move(other.m_alloc);
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    ~SimpleVector() 
    {
        clear();
        if(m_data) 
        { 
            std::allocator_traits<Alloc>::deallocate(m_alloc, m_data, m_capacity); 
        }
    }

    void push_back(const T& v) 
    {
        if(m_size >= m_capacity) 
        { 
            throw std::bad_alloc(); 
        }
        std::allocator_traits<Alloc>::construct(m_alloc, m_data + m_size, v);
        ++m_size;
    }

    void push_back(T&& v) 
    {
        if(m_size >= m_capacity) 
        { 
            throw std::bad_alloc(); 
        }
        std::allocator_traits<Alloc>::construct(m_alloc, m_data + m_size, std::move(v));
        ++m_size;
    }

    // Element access
    reference operator[](size_type i) 
    { 
        return m_data[i]; 
    }
    const_reference operator[](size_type i) const 
    { 
        return m_data[i]; 
    }

    reference at(size_type i) 
    {
        if(i >= m_size)
        {
            throw std::out_of_range("SimpleVector::at");
        }
        return m_data[i];
    }
    const_reference at(size_type i) const 
    {
        if(i >= m_size)
        {
            throw std::out_of_range("SimpleVector::at");
        }
        return m_data[i];
    }

    pointer data() noexcept { return m_data; }
    const_pointer data() const noexcept { return m_data; }

    // Iterators
    iterator begin() noexcept { return iterator(m_data); }
    iterator end() noexcept { return iterator(m_data + m_size); }
    const_iterator begin() const noexcept { return const_iterator(m_data); }
    const_iterator end() const noexcept { return const_iterator(m_data + m_size); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    // Capacity
    bool empty() const noexcept { return m_size == 0; }
    size_type size() const noexcept { return m_size; }
    size_type capacity() const noexcept { return m_capacity; }

    // Modifiers
    void clear() noexcept
    {
        for(size_type i = 0; i < m_size; ++i)
        {
            std::allocator_traits<Alloc>::destroy(m_alloc, m_data + i);
        }
        m_size = 0;
    }

private:
    Alloc m_alloc{};
    pointer m_data{nullptr};
    size_type m_size{0};
    size_type m_capacity{0};
};


} // namespace hw3

