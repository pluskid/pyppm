#ifndef _ARITHMETIC_ENCODER_H_
#define _ARITHMETIC_ENCODER_H_

#include "arithmetic_coding.h"
#include "bit_stream.h"

class ArithmeticEncoder
{
private:
    BitOutputStream *m_out;

    code_value m_low;
    code_value m_high;
    int m_bits_to_follow;

    // Output a bit plus following opposite bits
    void bit_plus_follow(int bit) {
        m_out->write(bit);
        m_out->write(!bit, m_bits_to_follow);
        m_bits_to_follow = 0;   // clear bits to follow
    }

public:
    ArithmeticEncoder(BitOutputStream *out=NULL)
        :m_out(out) {
        m_low = 0;
        m_high = Top_value;
        m_bits_to_follow = 0;
    }

    // Encode a symbol
    //  - low is the cumulative frequency below the symbol
    //  - high is the cumulative frequency of the symbol
    //  - total is the cumulative frequency of all symbols
    void encode(code_value low, code_value high, code_value total);

    // Finish encoding, output the remaining bits
    void finish_encoding() {
        m_bits_to_follow += 1;     // Output two bits that select the
        if (m_low < First_qtr) {   // quarterthat the current code range
            bit_plus_follow(0);    // contains
        } else {
            bit_plus_follow(1);
        }
    }
};

#endif /* _ARITHMETIC_ENCODER_H_ */
