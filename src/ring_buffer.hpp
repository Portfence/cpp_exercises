#pragma once

#include <cstddef>
#include <array>
#include <optional>
#include <utility>

template <class T, size_t SIZE>
class RingBuffer {
    static_assert(SIZE != 0, "ring buffer size can't be 0");
public:
    RingBuffer() : m_head(0), m_tail(0), m_size(0) {
    }

    void push(T&& data) {
        push_impl(std::move(data));
    }
  
    void push(const T& data) {
        push_impl(data);
    }

    std::optional<T> pop() {
        if (empty()) {
            return std::nullopt;
        }

        auto val = std::move(m_buffer[m_head]);
        m_head = (m_head + 1) % SIZE;
        
        m_size--;

        return val;
    }

    [[nodiscard]] bool empty() const {
        return m_size == 0;
    }

    [[nodiscard]] bool full() const {
        return m_size == SIZE;
    }

    [[nodiscard]] size_t size() const {
        return m_size;
    }
private:
    template<typename U>
    void push_impl(U&& data) {
        const bool was_full = full();

        m_buffer[m_tail] = std::forward<U>(data);

        m_tail = (m_tail + 1) % SIZE;

        if (was_full) {
            m_head = (m_head + 1) % SIZE;
        } else {
            m_size++;
        }
    }

    std::array<T, SIZE> m_buffer{};
    size_t m_head;
    size_t m_tail;
    size_t m_size;
};