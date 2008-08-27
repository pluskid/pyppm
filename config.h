#ifndef _CONFIG_H_
#define _CONFIG_H_

// Type of the symbol to encode
typedef unsigned char symbol_t;

// Type that can hold normal symbol as well as special symbols like
// ESC and EOF
typedef int wsymbol_t;

// Max number of contexts in the PPM model
#define Max_no_contexts 6


#endif /* _CONFIG_H_ */
