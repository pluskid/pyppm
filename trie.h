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
        m_escape = 0;
        m_sibling = NULL;
    }
};

template <typename T>
class Trie
{
private:
    TrieNode<T> *m_node;
    
public:
    Trie() :m_node(NULL) { }
    
    bool encode(Buffer<T> buf, int offset, symbol_t sym) {
        if (m_node == NULL) {
            TrieNode<T> *node = new TrieNode<T>(sym);
            node->m_escape++;

            for (int i = buf.length()-1; i >= offset; --i) {
                node = TrieNode<T>(sym, node);
            }
            m_node = node;
            
            
        } else {
        }
    }
};

#endif /* _TRIE_H_ */
