#include <cstdio>
#include "ppm_model.h"
#include "buffer.h"

using namespace std;

int main(int argc, char *argv[])
{
    ArithmeticEncoder *aenc = new ArithmeticEncoder(new BitOutputStream());
    PPMEncoder *penc = new PPMEncoder(aenc);

    char str[] = "abcccacbcacca";
    for (int i = 0; i < sizeof(str)/sizeof(str[0]); ++i) {
        penc->encode(str[i]);
    }
    return 0;
}
