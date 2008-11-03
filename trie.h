#ifndef _TRIE_H_
#define _TRIE_H_

#include <cassert>

#include "config.h"
#include "buffer.h"
#include "arithmetic_encoder.h"
#include "arithmetic_decoder.h"
#include "slab_allocator.h"

class TrieNode
{
private:
    symbol_t m_value;           // The value of this node
    unsigned short m_count;     // The scaled count
    unsigned short m_escape;    // The scaled number of escapes
    TrieNode *m_child;          // The first child
    TrieNode *m_sibling;        // The next sibling

    friend class Trie;

public:
    TrieNode(symbol_t value, TrieNode *child=NULL)
        :m_value(value), m_child(child) {
        m_count = 1;
        m_escape = 1;
        m_sibling = NULL;

        if (m_child != NULL)
            m_count++;
    }

    static void dump(TrieNode *node, FILE *f) {
        char flag = 0;
        if (node == NULL) {
            fwrite(&flag, sizeof(flag), 1, f);
        } else {
            flag = 1;
            fwrite(&flag, sizeof(flag), 1, f);

            write_int(node->m_value, f);
            write_int(node->m_count, f);
            write_int(node->m_escape, f);
            dump(node->m_child, f);
            dump(node->m_sibling, f);
        }
    }

    static TrieNode *load(FILE *f) {
        char flag;
        fread(&flag, sizeof(flag), 1, f);
        if (flag == 0) {
            return NULL;
        } else {
            TrieNode *node = (TrieNode *)operator new(sizeof(TrieNode));
            node->m_value = (symbol_t)read_int(f);
            node->m_count = (unsigned short)read_int(f);
            node->m_escape = (unsigned short)read_int(f);
            node->m_child = load(f);
            node->m_sibling = load(f);

            return node;
        }
    }

private:
    static void write_int(unsigned int n, FILE *f) {
        unsigned char ch;
        for (int i = 0; i < (int)sizeof(unsigned int); ++i) {
            ch = n & 0xFF;
            n >>= 8;
            fwrite(&ch, 1, 1, f);
        }
    }

    static unsigned int read_int(FILE *f) {
        unsigned char buf[sizeof(int)];
        fread(buf, 1, sizeof(buf), f);
        unsigned int n = 0;
        for (int i = (int)sizeof(int)-1; i >= 0; --i) {
            n <<= 8;
            n |= buf[i];
        }
        return n;
    }
};


//====================================================================
// The Trie used in PPM is a special Trie -- all leaves are on the
// same level.
//
// * Nodes with NULL child link are leaves. Leaf node maintain the
//   following properties:
//    - value: the symbol predicted by this leaf
//    - count: the number of occurance of this symbol in this context
//
// * The direct parent of a leaf node is a deterministic node, which
//   maintain the following properties:
//    - count: the sum of count property of all children
//    - escape: the number of escape happened in this context
//
// * The root node is only there for convenience of manipulation.
//
//  * Other nodes are normal nodes used to form the tree skeleton.
//====================================================================

class Trie
{
private:

    SlabAllocator<TrieNode> m_allocator;
    TrieNode *m_root;

    // Cache for updating model, set m_cache_parent to NULL to invalidate
    // the cache
    TrieNode *m_cache_parent;     // The cached parent node
    TrieNode *m_cache_child;      // The cached child node
    int m_cache_buf_idx;        // The cached index in the buffer
    
    TrieNode *create_node(const Buffer &buf, int offset, symbol_t sym) {
        // Leaf node
        TrieNode *node = new(m_allocator.allocate()) TrieNode(sym);

        for (int i = buf.length() - 1; i >= offset; --i) {
            node = new(m_allocator.allocate()) TrieNode(buf[i], node);
        }
        return node;
    }
    
public:
    Trie() :m_root(NULL), m_cache_parent(NULL) { }

    void dump(FILE *f) {
        TrieNode::dump(m_root, f);
    }
    void load(FILE *f) {
        m_root = TrieNode::load(f);
    }

    
    ////////////////////////////////////////////////////////////
    /// Encode symbol.
    ///
    /// Return true if predict successfully, false if escaped.
    ////////////////////////////////////////////////////////////
    template<typename Adapter>
    bool encode(ArithmeticEncoder<Adapter> *encoder, const Buffer &buf,
                int offset, symbol_t sym) {
        if (m_root == NULL) {

            // Context not initialized yet, simply escape
            return false;
        } else {
            TrieNode *parent = m_root;
            TrieNode *node = NULL;

            for (int i = offset; i < buf.length(); ++i) {
                node = parent->m_child;

                // Search for proper node
                while (node != NULL &&
                       node->m_value != buf[i])
                    node = node->m_sibling;

                if (node == NULL) {
                    // Set up cache for updating model
                    m_cache_parent = parent;
                    m_cache_child = node;
                    m_cache_buf_idx = i;

                    // Context not match, simply skip
                    return false;             // Escape
                } else {
                    parent = node;
                }
            }
            
            code_value cum = 0;
            bool res;
            node = parent->m_child;
            // Search for proper leaf
            while (node != NULL &&
                   node->m_value != sym) {
                cum += node->m_count;
                node = node->m_sibling;
            }

            m_cache_parent = parent;
            m_cache_child = node;
            m_cache_buf_idx = buf.length();

            if (node == NULL) {
                // No such node, predict failed
                // Encode the escape symbol
                assert(cum == (code_value)(parent->m_count-parent->m_escape));
                encoder->encode(cum, parent->m_count, parent->m_count);
                
                res = false;
            } else {
                // Predict success
                // Encode the symbol
                encoder->encode(cum, cum+node->m_count, parent->m_count);

                res = true;
            }

            if (parent->m_count >= Max_frequency) {
                scale_frequency(parent);
            }

            return res;
        }
    }

    template<typename Adapter>
    wsymbol_t decode(ArithmeticDecoder<Adapter> *decoder, const Buffer &buf, int offset) {
        code_value cum;

        if (m_root == NULL) {

            // Context not initialized yet, simply escape
            return ESC_symbol;
        } else {
            TrieNode *parent = m_root;
            TrieNode *node = NULL;
            
            for (int i = offset; i < buf.length(); ++i) {
                node = parent->m_child;

                // Search for proper node
                while (node != NULL &&
                       node->m_value != buf[i])
                    node = node->m_sibling;

                if (node == NULL) {
                    // Setup cache
                    m_cache_parent = parent;
                    m_cache_child = node;
                    m_cache_buf_idx = i;
                    
                    // Context not match, simply skip
                    return ESC_symbol;
                } else {
                    parent = node;
                }
            }

            cum = decoder->get_cum_freq(parent->m_count);
            code_value curr_cum = 0;
            node = parent->m_child;
            // Search for proper leaf
            while (node != NULL &&
                   curr_cum+node->m_count <= cum) {
                curr_cum += node->m_count;
                node = node->m_sibling;
            }

            m_cache_parent = parent;
            m_cache_child = node;
            m_cache_buf_idx = buf.length();
            
            if (node == NULL) {
                // No such node, predict failed, should be an escape
                cum = decoder->get_cum_freq(parent->m_count);
                assert(cum >= (code_value)parent->m_count-parent->m_escape);
                decoder->pop_symbol(parent->m_count-parent->m_escape,
                                    parent->m_count,
                                    parent->m_count);

                return ESC_symbol;
            } else {
                // Predict success
                decoder->pop_symbol(curr_cum, curr_cum+node->m_count,
                                    parent->m_count);
                return node->m_value;
            }

            if (parent->m_count >= Max_frequency) {
                scale_frequency(parent);
            }
        }
    }

    // Update the model, when some symbol is decoded, update
    // the related model
    void update_model(const Buffer &buf, int offset, symbol_t sym) {
        TrieNode *parent = NULL;
        TrieNode *node = NULL;
        if (m_cache_parent == NULL) {
            // Invalid cache
            if (m_root == NULL) {
                m_root = new(m_allocator.allocate()) TrieNode(0, create_node(buf, offset, sym));
            } else {
                parent = m_root;
                
                for (int i = offset; i < buf.length(); ++i) {
                    node = parent->m_child;

                    while (node != NULL &&
                           node->m_value != buf[i])
                        node = node->m_sibling;

                    if (node == NULL) {
                        node = create_node(buf, i, sym);

                        node->m_sibling = parent->m_child;
                        parent->m_child = node;
                        return;
                    } else {
                        parent = node;
                    }
                }

                node = parent->m_child;
                while (node != NULL &&
                       node->m_value != sym)
                    node = node->m_sibling;

                if (node == NULL) {
                    node = new(m_allocator.allocate()) TrieNode(sym);
                    node->m_sibling = parent->m_child;
                    parent->m_child = node;

                    parent->m_escape++;
                    parent->m_count += 2; // both escape and symbol
                } else {
                    node->m_count++;
                    parent->m_count++;
                }
            }
        } else {
            parent = m_cache_parent;

            if (m_cache_buf_idx == buf.length()) {
                if (m_cache_child == NULL) {
                    node = new(m_allocator.allocate()) TrieNode(sym);
                    node->m_sibling = m_cache_parent->m_child;
                    m_cache_parent->m_child = node;

                    m_cache_parent->m_escape++;
                    m_cache_parent->m_count += 2;
                } else {
                    m_cache_child->m_count++;
                    m_cache_parent->m_count++;
                }
            } else {
                node = create_node(buf, m_cache_buf_idx, sym);
                node->m_sibling = m_cache_parent->m_child;
                m_cache_parent->m_child = node;
            }
        }

        m_cache_parent = NULL;  // Invalidate cache

        if (parent && parent->m_count >= Max_frequency) {
            scale_frequency(parent);
        }
    }

    void scale_frequency(TrieNode *parent) 
    {
        int cum = 0;
        TrieNode *node = parent->m_child;
        TrieNode *prev = NULL;
        while (node != NULL) {
            if (node->m_count <= Min_frequency // Delete nodes with small frequency
                && (cum > 0 || node->m_sibling != NULL)) // But keep at least 1 node
            {
                if (prev == NULL) {
                    parent->m_child = node->m_sibling;
                } else {
                    prev->m_sibling = node->m_sibling;
                }
                m_allocator.release(node);
            } else {
                node->m_count = (node->m_count+Rescale_factor-1)/Rescale_factor;
                cum += node->m_count;
                prev = node;
            }
            
            node = node->m_sibling;
        }
        parent->m_escape = (parent->m_escape+Rescale_factor-1)/Rescale_factor;
        parent->m_count = cum + parent->m_escape;
    }
    
};

#endif /* _TRIE_H_ */
