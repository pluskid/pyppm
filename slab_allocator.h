#ifndef _SLAB_ALLOCATOR_H_
#define _SLAB_ALLOCATOR_H_

#include <new>
#include <algorithm>

////////////////////////////////////////////////////////////
// A slab allocator allocate memory for many objects of the
// same time.
//
// When memory is needed for an object, it first see whether
// there are free objects in the freelist. If not, it then
// look at the current block. It will request a new block if
// necessary. The new allocated block is not split into the
// freelist immediately. Only deallocated objects will be put
// into the freelist.
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

    Node  *m_blocks;            // Allocated blocks chained together

    char  *m_block;             // Current block used for allocating
    size_t m_block_remain;      // How many bytes remained for current block

    Node  *m_freelist;          // Released objects are chained here

    void get_new_block() {
        Node *block = (Node *)operator new(Block_size);

        // Link blocks for later release
        block->next = m_blocks;
        m_blocks = block;

        char *mem = (char *)block;

        // Skip the header used to link the blocks together
        size_t skip = std::max(sizeof(Node), sizeof(T));
        mem += skip;

        m_block = mem;
        m_block_remain = Block_size-skip;
    }
    
public:
    SlabAllocator()
        :m_blocks(NULL), m_block(NULL), m_block_remain(0), m_freelist(NULL) {
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
            if (m_block_remain < sizeof(T)) {
                get_new_block();
            }
            T *res = (T *)m_block;
            m_block += sizeof(T);
            m_block_remain -= sizeof(T);
            return res;
        } else {
            Node *res = m_freelist;
            m_freelist = m_freelist->next;
            return (T *)res;
        }
    }

    // Release
    void release(T *t) {
        Node *obj = (Node *)t;
        obj->next = m_freelist;
        m_freelist = obj;
    }
};

#endif /* _SLAB_ALLOCATOR_H_ */
