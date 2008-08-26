#ifndef _ARITHMETIC_CODING_H_
#define _ARITHMETIC_CODING_H_

#define Code_value_bits 16          /* Number of bits in a code value */
typedef unsigned long code_value;   /* Type of an arithmetic code
                                     * value */

#define Top_value (((code_value)1 << Code_value_bits) - 1) /* Largest
                                                            * code value */

#define First_qtr (Top_value/4+1)   /* Point after first quarter */
#define Half      (2*First_qtr)     /* Point after half          */
#define Third_qtr (3*First_qtr)     /* Point after third quarter */


#endif /* _ARITHMETIC_CODING_H_ */
