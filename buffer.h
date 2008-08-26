#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <cstring>

template <typename T, int N>
class Buffer
{
private:
    T m_buf[N+N];
    int m_offset;
    int m_length;

public:
    Buffer()
        :m_offset(0), m_length(0)
        { }

    // Append a value to the buffer, the buffer holds
    // at most N values, earlier values will be discarded
    // when necessary.
    void operator << (T value) {
        m_buf[m_offset++] = value;
        
        if (m_length < N) {
            m_length++;
        } else {
            if (m_offset == N+N) {
                std::memmove(m_buf, m_buf+N, N);
                m_offset = N;
            }
        }
    }

    // Get value from the buffer. The index should be
    // within [0, m_length) , and it is not checked.
    T operator[] (int pos) const {
        return m_buf[pos+m_offset-m_length];
    }

    // Get the length of the buffer
    int length() const { return m_length; }
};

#endif /* _BUFFER_H_ */
