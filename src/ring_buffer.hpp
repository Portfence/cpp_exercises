#pragma once

#include <array>
#include <cstddef>
#include <mutex>
#include <optional>
#include <utility>

template <class T, size_t SIZE> class RingBuffer {
    static_assert(SIZE != 0, "ring buffer size can't be 0");

  public:
    RingBuffer() : m_head(0), m_tail(0), m_size(0) {
    }

    void push(T &&data) {
        m_push_impl(std::move(data));
    }

    void push(const T &data) {
        m_push_impl(data);
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
    template <typename U> void m_push_impl(U &&data) {
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

template <class T, size_t SIZE> class AtomicRingBuffer {
  public:
    AtomicRingBuffer() = default;
    void push(T &&data) {
        std::lock_guard lock(m_mtx);
        m_rb.push(data);
    }

    void push(const T &data) {
        std::lock_guard lock(m_mtx);
        m_rb.push(data);
    }

    std::optional<T> pop() {
        std::lock_guard lock(m_mtx);
        return m_rb.pop();
    }

    [[nodiscard]] bool empty() const {
        std::lock_guard lock(m_mtx);
        return m_rb.empty();
    }

    [[nodiscard]] bool full() const {
        std::lock_guard lock(m_mtx);
        return m_rb.full();
    }

    [[nodiscard]] size_t size() const {
        std::lock_guard lock(m_mtx);
        return m_rb.size();
    }

  private:
    RingBuffer<T, SIZE> m_rb{};
    mutable std::mutex m_mtx;
};