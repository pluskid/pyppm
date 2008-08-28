#ifndef _PPM_MODEL_H_
#define _PPM_MODEL_H_

#include <cstring>

#include "config.h"
#include "buffer.h"
#include "trie.h"
#include "arithmetic_encoder.h"
#include "arithmetic_decoder.h"

class PPMModel
{
protected:
    typedef Trie<symbol_t> Trie_t;
    Trie_t m_contexts[Max_no_contexts+1];
    Buffer<symbol_t> m_buffer;
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
                if (m_contexts[ictx].encode(m_encoder, m_buffer, i, sym)) {
                    break;  // predict success
                }
            }
        }
    }
};

class PPMDecoder: public PPMModel
{
private:
    ArithmeticDecoder *m_decoder;
    
public:
    PPMDecoder(ArithmeticDecoder *decoder)
        :m_decoder(decoder) { }
    
    wsymbol_t decode() {
        int ictx = m_buffer.length();
        wsymbol_t symbol;

        if (ictx == 0) {
            // Uniformed frequency decode
        } else {
            for (int i = 0; ictx > 0; --ictx, ++i) {
                symbol = m_contexts[ictx].decode(m_decoder, m_buffer, i);
                if (symbol != ESC_symbol) {
                    // Update decoding model
                    for (int j = ictx+1, k = i-1;
                         j < m_buffer.length();
                         ++j, --k) {
                        m_contexts[j].update_model(m_buffer, k, symbol);
                    }
                    
                    return symbol;
                }
            }
        }
    }
};


#endif /* _PPM_MODEL_H_ */
