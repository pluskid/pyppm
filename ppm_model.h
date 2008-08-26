#ifndef _PPM_MODEL_H_
#define _PPM_MODEL_H_

#include <cstring>

#include "config.h"
#include "buffer.h"
#include "trie.h"

#define No_of_chars 256             /* Number of character(byte) symbols */
#define No_of_special_symbols 2     /* Number of special symbols */

#define EOF_symbol (No_of_chars+0)  /* the EOF symbol */
#define ESC_symbol (No_of_chars+1)  /* the Escape symbol */

#define No_of_symbols (No_of_chars+No_of_special_symbols)


class PPMModel
{
private:
    typedef TrieNode<symbol_t> Node_t;
    Node_t *m_contexts[Max_no_contexts+1];
    Buffer<symbol_t, Max_no_contexts> m_buffer;

    // Create a context
    Node_t *make_context(int offset, symbol_t symbol) {
        Node_t *node = new Node_t(symbol);
        for (int i = m_buffer.length()-1; i >= offset; --i) {
            node = new Node_t(m_buffer[i], node);
        }
        node;
    }

public:
    PPMModel() {
        std::memset(m_contexts, 0, sizeof(TrieNode *)*Max_no_contexts);
    }

    void encode(symbol_t symbol) {
        int ictx = m_buffer.length();
        
        if (ictx == 0) {
            // Uniformed frequency encode
        } else {
            for (int i = 0; ictx > 0; --ictx, ++i) {
                if (m_contexts[ictx] != NULL) {
                    if (!m_contexts[ictx]->encode(m_buffer, i, symbol)) {
                        break;  // predict success
                    }
                } else {
                    m_contexts[ictx] = make_context(i, symbol);
                }
            }
        }
    }
};

#endif /* _PPM_MODEL_H_ */
