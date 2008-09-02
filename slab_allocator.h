#ifndef _SLAB_ALLOCATOR_H_
#define _SLAB_ALLOCATOR_H_

#include <new>
#include <algorithm>

////////////////////////////////////////////////////////////
// A slab allocator allocate memory for many objects of the
// same time. It allocate a big block of Block_size at a time
// and split it into small chunks to form a freelist. Each
// time a new chunk is needed, it is fetched from the freelist.
// When no chunks available in the freelist, a new block is
// allocated.
//
// When the slab allocator is destroyed, all the blocks are
// freed. The allocator will NOT call either constructor
// or destructor.
////////////////////////////////////////////////////////////
template<typename T, size_t Block_size=4096>
class SlabAllocator
{
private:
    struct Node
    {
        Node *next;
    };

    Node *m_blocks;
    Node *m_freelist;

    void get_new_block() {
        Node *block = (Node *)operator new(Block_size);

        // Link blocks for later release
        block->next = m_blocks;
        m_blocks = block;

        char *mem = (char *)block;
        char *end = mem+Block_size;

        // Skip the header used to link the blocks together
        mem += std::max(sizeof(Node), sizeof(T));

        // Feed the remaining memory to free list
        while (mem + sizeof(T) <= end) {
            Node *obj = (Node *)mem;
            obj->next = m_freelist;
            m_freelist = obj;
            mem += sizeof(T);
        }
    }
    
public:
    SlabAllocator()
        :m_blocks(NULL), m_freelist(NULL) {
    }
    ~SlabAllocator() {
        Node *next;
        while (m_blocks != NULL) {
            next = m_blocks->next;
            delete m_blocks;
            m_blocks = next;
        }
    }

    // Allocate 
    T *allocate() {
        if (m_freelist == NULL) {
            get_new_block();
        }

        Node *res = m_freelist;
        m_freelist = m_freelist->next;
        return (T *)res;
    }

    // Release
    void release(T *t) {
        Node *obj = (Node *)t;
        obj->next = m_freelist;
        m_freelist = obj;
    }
};

#endif /* _SLAB_ALLOCATOR_H_ */
