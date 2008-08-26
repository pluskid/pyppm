#ifndef _TRIE_H_
#define _TRIE_H_

#include "config.h"
#include "buffer.h"

template <typename T>
class TrieNode
{
private:
    T m_value;                  // The value of this node
    unsigned short m_count;     // The scaled count
    unsigned short m_escape;    // The scaled number of escapes
    TrieNode<T> *m_child;       // The first child
    TrieNode<T> *m_sibling;     // The next sibling

    friend class Trie;

public:
    TrieNode(T value, TrieNode<T> *child=NULL)
        :m_value(value), :m_child(child) {
        m_count = 1;
        m_escape = 1;
        m_sibling = NULL;
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
    Node_t *m_node;
    
public:
    Trie() :m_root(NULL) { }
    
    // Encode symbol.
    //
    // Return true if predict successfully, false if escaped.
    bool encode(const Buffer<T> &buf, int offset, symbol_t sym) {
        if (m_root == NULL) {
            m_root = new Node_t(create_node(buf, offset, sym);
            
            // TODO: Encode the escape symbol

            return false;       // escape
        } else {
            Node_t *dtm_node = NULL;
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

                    // TODO: Encode escape symbol

                    return false;
                } else {
                    parent = node;
                }
            }
        }
    }

    Node_t *create_node(const Buffer<T> &buf, int offset, symbol_t sym) {
        // Leaf node
        Node_t *node = new Node_t(sym);

        for (int i = buf.length() - 1; i >= offset; --i) {
            node = new Node_t(sym, node);
        }
        return node;
    }
};

#endif /* _TRIE_H_ */
