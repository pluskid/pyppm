#ifndef _ARITHMETIC_DECODER_H_
#define _ARITHMETIC_DECODER_H_

#include "bit_stream.h"
#include "arithmetic_coding.h"

class ArithmeticDecoder
{
private:
    BitInputStream *m_in;

    code_value m_value;         // Currently-seen code value
    code_value m_low;           // low end of current region
    code_value m_high;          // high end of current region

public:
    ArithmeticDecoder(BitInputStream *in=NULL)
        :m_in(in) {
        m_low = 0;
        m_value = 0;
        m_high = Top_value;
    }

    // Start decoding
    void start_decoding() {
        // Input bits to fill the code value
        for (int i = 0; i < Code_value_bits; ++i) {
            m_value = (m_value<<1) + m_in->read();
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
            m_value = (m_value<<1) + m_in->read();
        }
    }
};

#endif /* _ARITHMETIC_DECODER_H_ */
