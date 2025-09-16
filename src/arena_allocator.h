#pragma once

#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>


namespace hw3 
{

// Arena allocator with block-based growth and no per-object free.
// - blockElems: number of elements per block (reserve step)
// - resizable: if false, total capacity is fixed to the first block; otherwise new blocks are added as needed
template <class T>
class ArenaAllocator
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // allocator rebind
    // for C++11+ allocator_traits::rebind_alloc rebound type
    template <class U>
    struct rebind
    {
        using other = ArenaAllocator<U>;
    };

    struct Params
    {
        size_type blockElems{1024};
        bool resizable{true};
    };

    // Default: 1024 elems/block, resizable
    ArenaAllocator() noexcept : m_params{}, m_state() 
    {
    }

    // !!! Fixed-capacity by default when explicitly specifying block size
    explicit ArenaAllocator(size_type blockElems, bool resizable = false) noexcept
        : m_params{Params{blockElems, resizable}}, m_state() 
    {
    } 

    template <class U>
    ArenaAllocator(const ArenaAllocator<U>& other) noexcept
        : m_params{Params{other.blockElems(), other.isResizable()}}, m_state() 
    {
    }

    const Params& params() const noexcept 
    {
        return m_params; 
    }

    size_type blockElems() const noexcept 
    {
        return m_params.blockElems; 
    }

    bool isResizable() const noexcept 
    {
        return m_params.resizable; 
    }

    pointer allocate(size_type n) 
    {
        // if nothing to allocate, return nullptr
        if(n == 0)
        {
            return nullptr;
        }
        ensureState();

        // if not enough space in current block and if resizable, grow
        if(hasRoom(n) == false)
        {
            if(m_state->params.resizable == false)
            {
                throw std::bad_alloc();
            }

            const size_type grow = n > m_state->params.blockElems ? n : m_state->params.blockElems;
            addBlock(grow);
        }

        // finally, allocate memory
        // move pointer from base to the first free element
        std::byte* pBase = reinterpret_cast<std::byte*>(m_state->blocks[m_state->current]);
        pointer pResult = reinterpret_cast<pointer>(pBase + m_state->usedInCurrent * sizeof(T));
        m_state->usedInCurrent += n;
        return pResult;
    }

    void deallocate(pointer /*p*/, size_type /*n*/) noexcept 
    {
        // no-op; memory released with state
    }

    template <class U, class... Args>
    void construct(U* p, Args&&... args) 
    {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }
    // as constuct uses placement-new, object lifetime is managed by the allocator (not tied to C++ scopes)
    template <class U>
    void destroy(U* p) 
    {
        p->~U(); 
    }

    size_type maxSize() const noexcept 
    {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    // Propagation traits: tell std containers how to transfer allocator during operations
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    // make visible for lookup by ADL: hidden friend idiom
    template <class U>
    friend bool operator==(const ArenaAllocator&, const ArenaAllocator<U>&) noexcept 
    {
        // Stateless w.r.t equality for simplicity
        return true;
    }
    // make visible for lookup by ADL: hidden friend idiom
    template <class U>
    friend bool operator!=(const ArenaAllocator& a, const ArenaAllocator<U>& b) noexcept 
    {
        // Ahtung!!! writing like this a != b causes an endless recursion
        return !(a == b);
    }

private:
    struct State
    {
        explicit State(const Params& p) : params(p) 
        {}
        ~State()
        {
            for(void* pBlock : blocks)
            {
                ::operator delete(pBlock, std::align_val_t(alignof(T)));
            }
        }

        Params params;
        std::vector<void*> blocks;
        std::vector<size_type> caps; // capacity, elements per block
        size_type current = static_cast<size_type>(-1);
        size_type usedInCurrent = 0;
    };

    void ensureState() 
    {
        if(m_state == nullptr)
        {
            m_state = std::make_shared<State>(m_params);
            addBlock(m_state->params.blockElems);
        }
    }

    bool hasRoom(size_type n) const noexcept 
    {
        if(m_state == nullptr || m_state->current == static_cast<size_type>(-1))
        {
            return false;
        }

        const size_type cap = m_state->caps[m_state->current];
        return m_state->usedInCurrent + n <= cap;
    }

    void addBlock(size_type elems) 
    {
        void* pMem = ::operator new(elems * sizeof(T), std::align_val_t(alignof(T)));
        m_state->blocks.push_back(pMem);
        m_state->caps.push_back(elems);
        m_state->current = m_state->blocks.size() - 1;
        m_state->usedInCurrent = 0;
    }

    Params m_params{};
    std::shared_ptr<State> m_state{};
};


} // namespace hw3

