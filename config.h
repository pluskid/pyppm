#ifndef _CONFIG_H_
#define _CONFIG_H_

////////////////////////////////////////////////////////////
// PPM Model parameters
////////////////////////////////////////////////////////////

// Type of the symbol to encode
typedef unsigned char symbol_t;

// Type that can hold normal symbol as well as special symbols like
// ESC and EOF
typedef int wsymbol_t;

// Max number of contexts in the PPM model
#define Max_no_contexts 6

#define No_of_chars 256             /* Number of character(byte) symbols */

#define EOF_symbol (No_of_chars+0)  /* the EOF symbol */
#define ESC_symbol (No_of_chars+1)  /* the Escape symbol */

// Include EOF_symbol but not ESC_symbol
#define No_of_symbols (No_of_chars+1)


////////////////////////////////////////////////////////////
// Arithmetic Codec parameters
////////////////////////////////////////////////////////////

#define Code_value_bits 16          /* Number of bits in a code value */
typedef unsigned long code_value;   /* Type of an arithmetic code
                                     * value */

#define Max_frequency ((1<<(Code_value_bits-2))-1) /* Max total frequency */
#define Min_frequency 512       /* Node with less freq will be removed */
#define Rescale_factor 32       /* By what factor does the frequency
                                 * be rescaled */

#define Top_value (((code_value)1 << Code_value_bits) - 1) /* Largest
                                                            * code value */

#define First_qtr (Top_value/4+1)   /* Point after first quarter */
#define Half      (2*First_qtr)     /* Point after half          */
#define Third_qtr (3*First_qtr)     /* Point after third quarter */


#endif /* _CONFIG_H_ */
