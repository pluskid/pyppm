#ifndef _ARITHMETIC_ENCODER_H_
#define _ARITHMETIC_ENCODER_H_

#include "config.h"

template<typename OutputAdapter>
class ArithmeticEncoder
{
private:
    // For encoding
    code_value m_low;
    code_value m_high;
    int m_bits_to_follow;

    // For buffered bit writing
    OutputAdapter &m_writer;
    int m_buffer;
    int m_bits_to_go;

    // Output a bit plus following opposite bits
    void bit_plus_follow(int bit) {
        write(bit);
        write(!bit, m_bits_to_follow);
        m_bits_to_follow = 0;   // clear bits to follow
    }

    ////////////////////////////////////////////////////////////
    // Buffered bit writing
    ////////////////////////////////////////////////////////////
    void write(int bit) {
        m_buffer >>= 1;
        if (bit)
            m_buffer |= 0x80;
        if (--m_bits_to_go == 0) {
            m_writer(m_buffer);
            m_bits_to_go = 8;
        }
    }

    // Write n bits at a time
    // 
    // NOTE n might be 0, in which case nothing
    // will be done.
    void write(int bit, int n) {
        if (n >= m_bits_to_go) {
            m_buffer >>= m_bits_to_go;
            if (bit)
                m_buffer |= ((bit<<m_bits_to_go)-1)<<(8-m_bits_to_go);
            m_writer(m_buffer);
            n -= m_bits_to_go;
            m_bits_to_go = 8;

            while (n >= 8) {
                if (bit)
                    m_writer((1<<8)-1);
                else
                    m_writer(0);
                n -= 8;
            }
        }

        m_buffer >>= n;
        if (bit)
            m_buffer |= ((bit<<n)-1)<<(8-n);
        m_bits_to_go -= n;
    }

    // Flush out the remaining bits if any
    void flush() {
        m_buffer >>= m_bits_to_go;
        m_bits_to_go = 8;
        m_writer(m_buffer);
    }
    
public:
    ArithmeticEncoder(OutputAdapter &writer)
        :m_low(0), m_high(Top_value), m_bits_to_follow(0),
         m_writer(writer), m_buffer(0), m_bits_to_go(8) {
    }

    // Encode a symbol
    //  - low is the cumulative frequency below the symbol
    //  - high is the cumulative frequency of the symbol
    //  - total is the cumulative frequency of all symbols
    void encode(code_value low, code_value high, code_value total) {
        code_value range = m_high-m_low+1;     // size of current code region
    
        m_high = m_low + (range*high)/total-1; // Narrow the region to
        m_low  = m_low + (range*low)/total;    // that allotted to this
        // symbol

        for (;;) {
            if (m_high < Half) {
                bit_plus_follow(0);            // Output 0 if in low half
            } else if (m_low >= Half) {
                bit_plus_follow(1);            // Output 1 if in high half
                m_low -= Half;
                m_high -= Half;
            } else if (m_low >= First_qtr &&
                       m_high < Third_qtr) {
                m_bits_to_follow++;            // Output an opposite bit
                m_low -= First_qtr;            // later if in middle half
                m_high -= First_qtr;
            } else {
                break;
            }

            m_low = m_low << 1;
            m_high = (m_high << 1) + 1;
        }
    }

    // Finish encoding, output the remaining bits
    void finish_encoding() {
        m_bits_to_follow += 1;     // Output two bits that select the
        if (m_low < First_qtr) {   // quarterthat the current code range
            bit_plus_follow(0);    // contains
        } else {
            bit_plus_follow(1);
        }
        flush();
    }
};

#endif /* _ARITHMETIC_ENCODER_H_ */
