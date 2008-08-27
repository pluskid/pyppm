#include "arithmetic_encoder.h"

void ArithmeticEncoder::encode(code_value low, code_value high,
                               code_value total)
{
    code_value range = m_high-m_low+1;     // size of current code region
    
    m_high = m_low + (range*high)/total-1; // Narrow the region to
    m_low  = m_low + (range*low)/total;    // that allotted to this
                                           // symbol

    for (;;) {
        if (high < Half) {
            bit_plus_follow(0);            // Output 0 if in low half
        } else if (low >= Half) {
            bit_plus_follow(1);            // Output 1 if in high half
            m_low -= Half;
            m_high -= Half;
        } else if (low >= First_qtr &&
                   high < Third_qtr) {
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
