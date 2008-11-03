#ifndef _PPM_MODEL_H_
#define _PPM_MODEL_H_

#include <cstring>

#include "config.h"
#include "buffer.h"
#include "trie.h"
#include "arithmetic_encoder.h"
#include "arithmetic_decoder.h"

struct PPMModel
{
    Trie m_contexts[Max_no_contexts+1];
    Buffer m_buffer;
    int m_refcount;

    PPMModel()
        :m_refcount(1) {
    }

    void incref() {
        ++m_refcount;
    }
    void decref() {
        if (--m_refcount == 0) {
            delete this;
        }
    }
    
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

class DefaultContextUpdater
{
public:
    void do_context_update(PPMModel *model, symbol_t sym) {
        model->update_contexts(sym);
    }
};
class NopeContextUpdater
{
public:
    void do_context_update(PPMModel *, symbol_t) {
    }
};

template<typename Adapter, typename ContextUpdater>
class PPMEncoder: public ContextUpdater
{
private:
    ArithmeticEncoder<Adapter> *m_encoder;
    PPMModel *m_model;
    
    void uni_encode(wsymbol_t sym) {
        m_encoder->encode(sym, sym+1, No_of_symbols);
    }

public:
    PPMEncoder(Adapter &ad)
        :m_encoder(new ArithmeticEncoder<Adapter>(ad)),
         m_model(new PPMModel()) {
    }

    PPMEncoder(Adapter &ad, PPMModel *model)
        :m_encoder(new ArithmeticEncoder<Adapter>(ad)),
         m_model(model) {
        m_model->incref();
    }

    ~PPMEncoder() {
        delete m_encoder;
        m_model->decref();
    }

    void start_encoding() {
        // Do nothing
    }
    
    void encode(wsymbol_t sym) {
        int ictx = m_model->m_buffer.length();

        if (ictx == 0) {
            uni_encode(sym);
        } else {
            
            for (int i = 0; ictx > 0; --ictx, ++i) {
                if (m_model->m_contexts[ictx].encode(m_encoder,
                                                     m_model->m_buffer, i, sym)) {
                    break;  // predict success
                }
            }

            if (ictx == 0)
                uni_encode(sym);
        }

        if (sym != EOF_symbol) {
            this->do_context_update(m_model, sym);
            m_model->m_buffer << sym;
        }
    }

    void finish_encoding() {
        encode(EOF_symbol);
        m_encoder->finish_encoding();
    }
};

template<typename Adapter, typename ContextUpdater>
class PPMDecoder: public ContextUpdater
{
private:
    ArithmeticDecoder<Adapter> *m_decoder;
    PPMModel *m_model;
    
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
        :m_decoder(new ArithmeticDecoder<Adapter>(ad)),
         m_model(new PPMModel()) {
    }

    PPMDecoder(Adapter &ad, PPMModel *model)
        :m_decoder(new ArithmeticDecoder<Adapter>(ad)),
         m_model(model) {
        m_model->incref();
    }

    ~PPMDecoder() {
        delete m_decoder;
        m_model->decref();
    }

    
    void start_decoding() {
        m_decoder->start_decoding();
    }
    
    wsymbol_t decode() {
        int ictx = m_model->m_buffer.length();
        wsymbol_t symbol;

        if (ictx == 0) {
            symbol = uni_decode();
        } else {
            for (int i = 0; ictx > 0; --ictx, ++i) {
                symbol = m_model->m_contexts[ictx].decode(m_decoder,
                                                          m_model->m_buffer, i);
                if (symbol != ESC_symbol) {
                    break;
                }
            }

            if (ictx == 0)
                symbol = uni_decode();
        }

        if (symbol != EOF_symbol) {
            this->do_context_update(m_model, symbol);
            m_model->m_buffer << symbol;
        }
        return symbol;
    }

    void finish_decoding() {
        // Do nothing
    }
};


#endif /* _PPM_MODEL_H_ */
