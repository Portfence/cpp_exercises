#pragma once

#include <unordered_map>
#include <list>
#include <optional>
#include <cstddef>

template<typename Key, typename Value>
class LRUCache{
public:
    LRUCache(size_t capacity): m_map(capacity), m_list({}) {}

    void put(const Key& key, const Value& value) {
        const bool exists = m_map.find(key) != m_map.end();

        if (exists && (key == m_list.front())) {
            // element is already at the front – just update the value in the map
            m_map.insert(std::make_pair(key, value));
        } else if (exists) {
            // move to front and update value
            m_list.remove(key);
            m_list.push_front(key);
            m_map.insert(std::make_pair(key, value));
        } else if (m_map.size() == m_map.bucket_count()) {
            // cache full and element does not exist in cache – trigger eviction of least recently used element:
            auto element_to_remove = m_list.back();
            m_map.erase(element_to_remove); // Key,Value is removed from m_map
            m_list.pop_back(); // TODO: check if can do auto key = *m_list.pop_back() ?
        } else {
            // cache not full and element does not exist – add to map and front of list
            m_list.push_front(key);
            m_map.insert(std::make_pair(key, value));
        }
    }
    
    [[nodiscard]] std::optional<Value> get(const Key& key) {
        if (auto it = m_map.find(key); it != m_map.end()) {
            m_list.remove(key);
            m_list.push_front(key);
            return it->second;
        }
        return std::nullopt;
    }

private:
    std::unordered_map<Key, Value> m_map;
    std::list<Key> m_list;
};