#include <cstdio>
#include "ppm_model.h"
#include "buffer.h"

using namespace std;

void dump_buffer(const Buffer<char> &buf)
{
    bool first = true;
    printf("buf = [");
    for (int i = 0; i < buf.length(); ++i) {
        if (!first) {
            printf(", ");
        }
        first = false;

        printf("'%c'", buf[i]);
    }
    printf("]\n");
}

int main(int argc, char *argv[])
{
    Buffer<char> buf;

    buf << 'a';
    dump_buffer(buf);

    buf << 'b';
    buf << 'c';
    dump_buffer(buf);

    for (int i = 0; i < 9; ++i)
        buf << '0' + i;
    dump_buffer(buf);

    return 0;
}
