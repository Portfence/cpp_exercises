#pragma once

#include <cassert>
#include <cstddef>
#include <list>
#include <optional>
#include <unordered_map>

template <typename Key, typename Value> class LRUCache {
  public:
    LRUCache(size_t capacity) : m_capacity(capacity) {
        assert(capacity > 0);
    }

    void put(const Key& key, const Value& value) {
        if (auto it = m_map.find(key); it != m_map.end()) {
            // key exists: move to front and update value
            auto node_it = it->second;
            m_list.splice(m_list.begin(), m_list, node_it);
            node_it->second = value;
            return;
        }

        if (m_map.size() == m_capacity) {
            // trigger eviction of least used entry
            auto& back_node = m_list.back();
            m_map.erase(back_node.first);
            m_list.pop_back();
        }

        m_list.emplace_front(key, value);
        m_map.emplace(key, m_list.begin());
    }

    [[nodiscard]] const Value* get(const Key& key) {
        if (auto it = m_map.find(key); it != m_map.end()) {
            m_list.splice(m_list.begin(), m_list, it->second);
            return &(it->second->second);
        }
        return nullptr;
    }

  private:
    std::list<std::pair<Key, Value>> m_list{};
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> m_map{};
    const size_t m_capacity;
};
