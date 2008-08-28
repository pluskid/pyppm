#ifndef _PPM_MODEL_H_
#define _PPM_MODEL_H_

#include <cstring>

#include "config.h"
#include "buffer.h"
#include "trie.h"
#include "arithmetic_encoder.h"
#include "arithmetic_decoder.h"

#define No_of_chars 256             /* Number of character(byte) symbols */
#define No_of_special_symbols 2     /* Number of special symbols */

#define EOF_symbol (No_of_chars+0)  /* the EOF symbol */
#define ESC_symbol (No_of_chars+1)  /* the Escape symbol */

#define No_of_symbols (No_of_chars+No_of_special_symbols)


class PPMModel
{
private:
    typedef Trie<symbol_t> Trie_t;
    Trie_t m_contexts[Max_no_contexts+1];
    Buffer<symbol_t, Max_no_contexts> m_buffer;

public:
    PPMModel() {
        std::memset(m_contexts, 0, sizeof(TrieNode *)*Max_no_contexts);
    }
};

class PPMEncoder: public PPMModel
{
private:
    ArithmeticEncoder *m_encoder;

public:
    PPMEncoder(ArithmeticEncoder *encoder)
        :m_encoder(encoder) { }
    
    void encode(symbol_t sym) {
        int ictx = m_buffer.length();
        
        if (ictx == 0) {
            // Uniformed frequency encode
        } else {
            for (int i = 0; ictx > 0; --ictx, ++i) {
                if (m_contexts[ictx]->encode(m_encoder, m_buffer, i, sym)) {
                    break;  // predict success
                }
            }
        }
    }
};

class PPMDecoder: public PPMModel
{
public:
    wsymbol_t decode() {
        int ictx = m_buffer.length();
        wsymbol_t symbol;

        if (ictx == 0) {
            // Uniformed frequency decode
        } else {
            for (int i = 0; ictx > 0; --ictx, ++i) {
                symbol = m_contexts[ictx].decode(m_buffer, i);
                if (symbol != ESC_symbol) {
                    // Update decoding model
                    for (int j = ictx+1, k = i-1;
                         j < m_buffer.length();
                         ++j, --k) {
                        m_contexts[j].update_model(m_buffer, k, sym);
                    }
                    
                    return symbol;
                }
            }
        }
    }
};


#endif /* _PPM_MODEL_H_ */
