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

    // Initialize the 0th context
    // That is where all symbols are assigned
    // with 1 occurance
    void init_context0() {
        m_contexts[0].make_uni_trie(0, No_of_chars-1);
    }
};

class PPMEncoder: public PPMModel
{
private:
    ArithmeticEncoder *m_encoder;

public:
    PPMEncoder(ArithmeticEncoder *encoder)
        :m_encoder(encoder) { }

    void start_encoding() {
        init_context0();
    }
    
    void encode(symbol_t sym) {
        int ictx = m_buffer.length();
        
        for (int i = 0; ictx >= 0; --ictx, ++i) {
            if (m_contexts[ictx].encode(m_encoder, m_buffer, i, sym)) {
                break;  // predict success
            }
        }
    }

    void finish_encoding() {
        m_encoder->finish_encoding();
    }
};

class PPMDecoder: public PPMModel
{
private:
    ArithmeticDecoder *m_decoder;
    
public:
    PPMDecoder(ArithmeticDecoder *decoder)
        :m_decoder(decoder) { }

    void start_decoding() {
        init_context0();
        m_decoder->start_decoding();
    }
    
    wsymbol_t decode() {
        int ictx = m_buffer.length();
        wsymbol_t symbol;

        for (int i = 0; ictx >= 0; --ictx, ++i) {
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

        // Should not reach here
        assert(0);
    }

    void finish_decoding() {
        // Do nothing
    }
};


#endif /* _PPM_MODEL_H_ */
