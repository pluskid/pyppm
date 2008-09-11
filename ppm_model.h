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
    Trie m_contexts[Max_no_contexts+1];
    Buffer m_buffer;

    void update_contexts(symbol_t sym) {
        int ictx = 1;
        int offset = m_buffer.length() - 1;
        while (offset >= 0) {
            m_contexts[ictx].update_model(m_buffer, offset, sym);
            offset--;
            ictx++;
        }
    }
};

template<typename Adapter>
class PPMEncoder: public PPMModel
{
private:
    ArithmeticEncoder<Adapter> *m_encoder;

    void uni_encode(wsymbol_t sym) {
        m_encoder->encode(sym, sym+1, No_of_symbols);
    }

public:
    PPMEncoder(Adapter &ad)
        :m_encoder(new ArithmeticEncoder<Adapter>(ad)) {
    }

    ~PPMEncoder() {
        delete m_encoder;
    }

    void start_encoding() {
        // Do nothing
    }
    
    void encode(wsymbol_t sym) {
        int ictx = m_buffer.length();

        if (ictx == 0) {
            uni_encode(sym);
        } else {
            
            for (int i = 0; ictx > 0; --ictx, ++i) {
                if (m_contexts[ictx].encode(m_encoder, m_buffer, i, sym)) {
                    break;  // predict success
                }
            }

            if (ictx == 0)
                uni_encode(sym);
        }

        if (sym != EOF_symbol) {
            update_contexts(sym);
            m_buffer << sym;
        }
    }

    void finish_encoding() {
        encode(EOF_symbol);
        m_encoder->finish_encoding();
    }
};

template<typename Adapter>
class PPMDecoder: public PPMModel
{
private:
    ArithmeticDecoder<Adapter> *m_decoder;

    wsymbol_t uni_decode() {
        code_value cum;
        wsymbol_t sym;
        cum = m_decoder->get_cum_freq(No_of_symbols);
        assert(cum >= 0 && cum < No_of_symbols);
        m_decoder->pop_symbol(cum, cum+1, No_of_symbols);
        sym = cum;

        return sym;
    }
    
public:
    PPMDecoder(Adapter &ad)
        :m_decoder(new ArithmeticDecoder<Adapter>(ad)) {
    }

    ~PPMDecoder() {
        delete m_decoder;
    }

    
    void start_decoding() {
        m_decoder->start_decoding();
    }
    
    wsymbol_t decode() {
        int ictx = m_buffer.length();
        wsymbol_t symbol;

        if (ictx == 0) {
            symbol = uni_decode();
        } else {
            for (int i = 0; ictx > 0; --ictx, ++i) {
                symbol = m_contexts[ictx].decode(m_decoder, m_buffer, i);
                if (symbol != ESC_symbol) {
                    break;
                }
            }

            if (ictx == 0)
                symbol = uni_decode();
        }

        if (symbol != EOF_symbol) {
            update_contexts(symbol);
            m_buffer << symbol;
        }
        return symbol;
    }

    void finish_decoding() {
        // Do nothing
    }
};


#endif /* _PPM_MODEL_H_ */
