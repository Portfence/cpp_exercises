#pragma once

#include <cstddef>

template <typename T> class SharedPtr {
  public:
    explicit SharedPtr(T* ptr = nullptr) : m_data(ptr) {
        if (ptr) {
            m_ref_count = new size_t(1);
        } else {
            m_ref_count = nullptr;
        }
    }

    SharedPtr(const SharedPtr& ptr) : m_data(ptr.m_data), m_ref_count(ptr.m_ref_count) {
        if (m_ref_count) {
            (*m_ref_count)++;
        }
    }

    SharedPtr(SharedPtr&& ptr) noexcept : m_data(ptr.m_data), m_ref_count(ptr.m_ref_count) {
        // move should leave ptr empty after transferring ownership
        ptr.m_data = nullptr;
        ptr.m_ref_count = nullptr;
    }

    SharedPtr& operator=(const SharedPtr& ptr) {
        if (this == &ptr) {
            return *this;
        }
        m_release();

        m_data = ptr.m_data;
        m_ref_count = ptr.m_ref_count;
        if (m_ref_count) {
            (*m_ref_count)++;
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& ptr) noexcept {
        if (this == &ptr) {
            return *this;
        }

        m_release();
        m_data = ptr.m_data;
        m_ref_count = ptr.m_ref_count;

        ptr.m_data = nullptr;
        ptr.m_ref_count = nullptr;

        return *this;
    }

    ~SharedPtr() {
        m_release();
    }

    T& operator*() {
        return *(this->m_data);
    }

    T* operator->() {
        return this->m_data;
    }

    const T& operator*() const {
        return *m_data;
    }

    const T* operator->() const {
        return m_data;
    }

    T* get() const noexcept {
        return m_data;
    }

    size_t use_count() const noexcept {
        return m_ref_count ? *m_ref_count : 0;
    }

    explicit operator bool() const noexcept {
        return m_data != nullptr;
    }

    void reset() noexcept {
        m_release();
        m_data = nullptr;
        m_ref_count = nullptr;
    }

  private:
    void m_release() {
        if (!m_ref_count) {
            return;
        }

        if (--(*m_ref_count) == 0) {
            delete m_data;
            delete m_ref_count;
        }
    }

    T* m_data;
    size_t* m_ref_count;
};
