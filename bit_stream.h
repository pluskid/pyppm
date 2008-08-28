#ifndef _BIT_STREAM_H_
#define _BIT_STREAM_H_

#include <cstdio>

class BitOutputStream
{
public:
    void write(int bit) {
        std::printf("%d\n", bit);
    }

    // Write n bits at a time
    // 
    // NOTE n might be 0, in which case nothing
    // will be done.
    void write(int bit, int n) {
        for (int i = 0; i < n; ++i)
            std::printf("%d\n", bit);
    }
};

class BitInputStream
{
public:
    int read() {
        return 0;
    }
};

#endif /* _BIT_STREAM_H_ */
