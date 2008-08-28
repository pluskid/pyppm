#include <cstdio>
#include "ppm_model.h"
#include "buffer.h"

using namespace std;

int main(int argc, char *argv[])
{
    vector<int> vec;
    ArithmeticEncoder *aenc = new ArithmeticEncoder(new BitOutputStream(vec));
    PPMEncoder *penc = new PPMEncoder(aenc);

    char str[] = "abcccacbcacca";
    penc->start_encoding();
    for (int i = 0; i < sizeof(str)/sizeof(str[0]); ++i) {
        penc->encode(str[i]);
    }
    penc->finish_encoding();

    printf("------------------------------------\n");
    ArithmeticDecoder *adec = new ArithmeticDecoder(new BitInputStream(vec));
    PPMDecoder *pdec = new PPMDecoder(adec);
    pdec->start_decoding();
    for (int i = 0; i < sizeof(str)/sizeof(str[0]); ++i) {
        printf("[%d]", pdec->decode());
    }
    pdec->finish_decoding();
    printf("\n");
    return 0;
}
