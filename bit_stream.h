#ifndef _BIT_STREAM_H_
#define _BIT_STREAM_H_

#include <vector>
#include <cstdio>

class BitOutputStream
{
public:
    std::vector<int> &m_vec;
    
    BitOutputStream(std::vector<int> &vec)
        :m_vec(vec) { }
    
    void write(int bit) {
        m_vec.push_back(bit);
    }

    // Write n bits at a time
    // 
    // NOTE n might be 0, in which case nothing
    // will be done.
    void write(int bit, int n) {
        for (int i = 0; i < n; ++i)
            write(bit);
    }
};

class BitInputStream
{
public:
    std::vector<int> &m_vec;

    BitInputStream(std::vector<int> &vec)
        :m_vec(vec) { }
    
        
    int read() {
        if (m_vec.empty())
            return -1;
        int res = m_vec[0];
        m_vec.erase(m_vec.begin());
        return res;
    }
};

#endif /* _BIT_STREAM_H_ */
