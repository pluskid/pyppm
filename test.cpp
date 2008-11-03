#include <cstdio>
#include "ppm_model.h"
#include "buffer.h"
#include "io_adapter.h"

using namespace std;

int main(int argc, char *argv[])
{
    FILE *fout = fopen("encoded.txt", "wb");
    FILE *forig = fopen("input.txt", "rb");

    FileOutputAdapter foad(fout);

    PPMEncoder<FileOutputAdapter, DefaultContextUpdater> *penc =
        new PPMEncoder<FileOutputAdapter, DefaultContextUpdater>(foad);

    printf("Encoding...\n");
    penc->start_encoding();
    for (int ch = fgetc(forig); ch != EOF; ch = fgetc(forig))
        penc->encode(ch);
    penc->finish_encoding();

    fclose(fout);
    fclose(forig);
    printf("------------------------------------\n");
    
    FILE *fin = fopen("encoded.txt", "rb");
    FILE *fnew = fopen("decoded.txt", "wb");

    FileInputAdapter fiad(fin);
    
    PPMDecoder<FileInputAdapter, DefaultContextUpdater> *pdec =
        new PPMDecoder<FileInputAdapter, DefaultContextUpdater>(fiad);
    pdec->start_decoding();
    wsymbol_t sym;
    for (;;) {
        sym = pdec->decode();
        if (sym == EOF_symbol)
            break;
        fputc(sym, fnew);
    }
    pdec->finish_decoding();
    fclose(fin);
    fclose(fnew);
    
    return 0;
}
