#ifndef _CONFIG_H_
#define _CONFIG_H_

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


#endif /* _CONFIG_H_ */
