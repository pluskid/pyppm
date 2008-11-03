#ifndef _ARITHMETIC_DECODER_H_
#define _ARITHMETIC_DECODER_H_

#include "config.h"

template<typename InputAdapter>
class ArithmeticDecoder
{
private:
    // For Decoding
    code_value m_value;         // Currently-seen code value
    code_value m_low;           // low end of current region
    code_value m_high;          // high end of current region

    // For buffered bit reading
    InputAdapter &m_reader;
    int m_buffer;
    int m_bits_to_go;
        
    int read() {
        int res;

        if (m_bits_to_go == 0) {
            m_buffer = m_reader();
            if (m_buffer == EOF) { // NOTE: it might be indicating a
                m_buffer = 0;      // problem when reading too many EOF
            }
            m_bits_to_go = 8;
        }

        res = m_buffer & 1;
        m_buffer >>= 1;
        m_bits_to_go--;

        return res;
    }

public:
    ArithmeticDecoder(InputAdapter &reader)
        :m_value(0), m_low(0), m_high(Top_value),
         m_reader(reader), m_buffer(0), m_bits_to_go(0) {
    }
    
    // Start decoding
    void start_decoding() {
        // Input bits to fill the code value
        for (int i = 0; i < Code_value_bits; ++i) {
            m_value = (m_value<<1) + read();
        }
    }

    // Get the cumulative frequence for the next symbol
    code_value get_cum_freq(code_value total) {
        return ((m_value-m_low+1)*total - 1) / (m_high-m_low+1);
    }

    // Remove the symbol represented by low, high and total
    // and fetch new bits if necessary
    void pop_symbol(code_value low, code_value high, code_value total) {
        code_value range = m_high-m_low + 1;

        m_high = m_low + (range*high)/total - 1;
        m_low  = m_low + (range*low)/total;

        for (;;) {
            if (m_high < Half) {
                // Do nothing
            } else if (m_low >= Half) {
                m_value -= Half;
                m_low -= Half;
                m_high -= Half;
            } else if (m_low >= First_qtr && m_high < Third_qtr) {
                m_value -= First_qtr;
                m_low -= First_qtr;
                m_high -= First_qtr;
            } else {
                break;
            }

            m_low = m_low<<1;
            m_high = (m_high<<1) + 1;
            m_value = (m_value<<1) + read();
        }
    }
};

#endif /* _ARITHMETIC_DECODER_H_ */
