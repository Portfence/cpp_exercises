#pragma once

#include <unordered_map>
#include <list>
#include <optional>
#include <cstddef>

template<typename Key, typename Value>
class LRUCache{
public:
    LRUCache(size_t capacity): m_map({}), m_list({}), m_capacity(capacity) {}

    void put(const Key& key, const Value& value) {

    }
    
    [[nodiscard]] const Value* get(const Key& key) {
        if (auto it = m_map.find(key); it != m_map.end()) {
            m_list.splice(m_list.begin(), m_list, it->second);
            return &(it->second->second); 
        }
        return nullptr;
    }
private:
    std::list<std::pair<Key, Value>> m_list;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> m_map;
    size_t m_capacity;
};