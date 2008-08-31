#ifndef _TRIE_H_
#define _TRIE_H_

#include <cassert>

#include "config.h"
#include "buffer.h"
#include "arithmetic_encoder.h"
#include "arithmetic_decoder.h"

// Forward declaraing
template <typename T>
class Trie;


template <typename T>
class TrieNode
{
private:
    T m_value;                  // The value of this node
    unsigned short m_count;     // The scaled count
    unsigned short m_escape;    // The scaled number of escapes
    TrieNode<T> *m_child;       // The first child
    TrieNode<T> *m_sibling;     // The next sibling

    friend class Trie<T>;

public:
    TrieNode(T value, TrieNode<T> *child=NULL)
        :m_value(value), m_child(child) {
        m_count = 1;
        m_escape = 1;
        m_sibling = NULL;

        if (m_child != NULL)
            m_count++;
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

template <typename T>
class Trie
{
private:
    typedef TrieNode<T> Node_t;
    Node_t *m_root;
    
    Node_t *create_node(const Buffer<T> &buf, int offset, symbol_t sym) {
        // Leaf node
        Node_t *node = new Node_t(sym);

        for (int i = buf.length() - 1; i >= offset; --i) {
            node = new Node_t(sym, node);
        }
        return node;
    }
    
public:
    Trie() :m_root(NULL) { }

    
    ////////////////////////////////////////////////////////////
    /// Encode symbol.
    ///
    /// Return true if predict successfully, false if escaped.
    ////////////////////////////////////////////////////////////
    bool encode(ArithmeticEncoder *encoder, const Buffer<T> &buf,
                int offset, symbol_t sym) {
        if (m_root == NULL) {

            // Newly created trie, occurance and escape
            // should be both 1
            encoder->encode(1, 2, 2);  // Encode the escape symbol
            return false;              // Escape
        } else {
            Node_t *parent = m_root;
            Node_t *node = NULL;

            for (int i = offset; i < buf.length(); ++i) {
                node = parent->m_child;

                // Search for proper node
                while (node != NULL &&
                       node->m_value != buf[i])
                    node = node->m_sibling;

                if (node == NULL) {
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

            if (node == NULL) {
                // No such node, predict failed
                // Encode the escape symbol
                encoder->encode(cum, parent->m_count, parent->m_count);
                
                // Increase the number of occurance of escape symbol
                parent->m_escape++;
                parent->m_count++;
                res = false;
            } else {
                // Predict success
                // Encode the symbol
                encoder->encode(cum, cum+node->m_count, parent->m_count);

                node->m_count++;   // node count
                parent->m_count++; // parent total count
                res = true;
            }

            // TODO: if total count exceed threshold,
            // rescale
            return res;
        }
    }

    wsymbol_t decode(ArithmeticDecoder *decoder, const Buffer<T> &buf, int offset) {
        code_value cum;

        if (m_root == NULL) {
            
            // Should be an escape symbol
            cum = decoder->get_cum_freq(2);
            assert(cum >= 1);
            decoder->pop_symbol(1, 2, 2); // Pop the escape symbol
            return ESC_symbol;
        } else {
            Node_t *parent = m_root;
            Node_t *node = NULL;
            
            for (int i = offset; i < buf.length(); ++i) {
                node = parent->m_child;

                // Search for proper node
                while (node != NULL &&
                       node->m_value != buf[i])
                    node = node->m_sibling;

                if (node == NULL) {
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

            if (node == NULL) {
                // No such node, predict failed, should be an escape
                cum = decoder->get_cum_freq(parent->m_count);
                assert(cum >= (code_value)parent->m_count-parent->m_escape);
                decoder->pop_symbol(parent->m_count-parent->m_escape,
                                    parent->m_count,
                                    parent->m_count);

                parent->m_escape++;
                parent->m_count++;
                
                return ESC_symbol;
            } else {
                // Predict success
                decoder->pop_symbol(curr_cum, curr_cum+node->m_count,
                                    parent->m_count);
                parent->m_count++;
                node->m_count++;
                return node->m_value;
            }

            // TODO: if total count exceed threshold
            // rescale
        }
    }

    // Update the model, when some symbol is decoded, update
    // the related model
    void update_model(const Buffer<T> &buf, int offset, symbol_t sym) {
        if (m_root == NULL) {
            m_root = new Node_t(0, create_node(buf, offset, sym));
        } else {
            Node_t *parent = m_root;
            Node_t *node = NULL;

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
                node = new Node_t(sym);
                node->m_sibling = parent->m_child;
                parent->m_child = node;

                parent->m_escape++;
                parent->m_count++;
            } else {
                node->m_count++;
                parent->m_count++;
            }

            // TODO: if total count exceed threshold
            // rescale
        }
    }
};

#endif /* _TRIE_H_ */
