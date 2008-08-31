#include <cstdio>
#include "ppm_model.h"
#include "buffer.h"

using namespace std;

int main(int argc, char *argv[])
{
    vector<int> vec;
    ArithmeticEncoder *aenc = new ArithmeticEncoder(new BitOutputStream(vec));
    PPMEncoder *penc = new PPMEncoder(aenc);

    symbol_t str[] = "This is true!";
    penc->start_encoding();
    for (int i = 0; i < sizeof(str)/sizeof(str[0]); ++i) {
        penc->encode(str[i]);
    }
    penc->finish_encoding();

    printf("Encoded %d bits in %d bits", sizeof(str)*8, vec.size());
    
    printf("------------------------------------\n");
    ArithmeticDecoder *adec = new ArithmeticDecoder(new BitInputStream(vec));
    PPMDecoder *pdec = new PPMDecoder(adec);
    pdec->start_decoding();
    wsymbol_t sym;
    for (;;) {
        sym = pdec->decode();
        if (sym == EOF_symbol)
            break;
        printf("%c", sym);
    }
    pdec->finish_decoding();
    printf("\n%d\n", vec.size());
    return 0;
}
