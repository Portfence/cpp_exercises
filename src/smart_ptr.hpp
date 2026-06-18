#pragma once

#include <cstddef>

template <typename T> class SharedPtr {
  public:
    explicit SharedPtr(T* ptr = nullptr) : m_data(ptr) {
        m_ref_count = new size_t(1);
    }

    SharedPtr(const SharedPtr& ptr) : m_data(ptr.m_data), m_ref_count(ptr.m_ref_count) {
        (*m_ref_count)++;
    }

    SharedPtr& operator=(const SharedPtr& ptr) {
        if (this == &ptr) {
            return *this;
        }
        m_destruct();

        m_data = ptr.m_data;
        m_ref_count = ptr.m_ref_count;
        (*m_ref_count)++;
        return *this;
    }

    ~SharedPtr() {
        m_destruct();
    }

    T& operator*() {
        return *(this->m_data);
    }

    T* operator->() {
        return this->m_data;
    }

  private:
    void m_destruct() {
        (*m_ref_count)--;
        if ((*m_ref_count) == 0 && m_data) {
            delete m_data;
        }
    }

    T* m_data;
    size_t* m_ref_count;
};
