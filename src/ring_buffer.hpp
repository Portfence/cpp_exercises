#pragma once

#include <cstddef>
#include <array>
#include <optional>

template <class T, size_t SIZE>
class RingBuffer {
public:
    RingBuffer() : m_buffer(std::array<T, SIZE>{}), m_head(0), m_tail(0) {
        static_assert(SIZE != 0, "ring buffer size can't be 0");
    }

    void push(const T& data) {
        push(std::move(data));
    }

    void push(T&& data) {
        if (m_tail == (m_buffer.size() - 1)) {
            m_buffer[m_tail] = data;
            m_tail = 0;
        } else {
            m_buffer[m_tail++] = data;
        }
        

        if (m_tail == m_head) {
            m_head++;
        }
    }

    std::optional<T> pop() {
        if (empty()) {
            return std::nullopt;
        }
        return m_buffer[m_head++];
    }

    [[nodiscard]] bool empty() const {
        return size() == 0;
    }

    [[nodiscard]] bool full() const {
        return size() == m_buffer.size();
    }

    [[nodiscard]] size_t size() const {
        if (m_tail > m_head) return (m_tail - m_head);
        else if (m_tail < m_head) return m_buffer.size() - (m_head - m_tail) + 1 ;
        else return 0;
    }
private:
    std::array<T, SIZE> m_buffer;
    size_t m_head;
    size_t m_tail;
};