#include "lru_cache.hpp"
#include "ring_buffer.hpp"

#include <iostream>
#include <string>

template <class T, size_t SIZE> void print_state(const AtomicRingBuffer<T, SIZE> &rb) {
    using std::cout;
    using std::endl;

    cout << "size: " << rb.size() << endl;
    cout << "full: " << rb.full() << endl;
    cout << "empty: " << rb.empty() << endl;
    cout << endl;
}

void test_ring_buffer() {
    auto rb = RingBuffer<int, 5>{};
    rb.pop();
    auto arb = AtomicRingBuffer<int, 5>{};
    arb.pop();
    print_state(arb);
    for (int i = 0; i < 10; i++) {
        arb.push(i);
        print_state(arb);
    }

    for (int i = 0; i < 6; i++) {
        if (const auto val = arb.pop()) {
            std::cout << "val: " << *val << std::endl;
        }
    }
}

void test_lru_cache() {
    const auto capacity = 5;
    auto lru = LRUCache<int, std::string>(capacity);

    lru.put(1, std::string("hello"));
    lru.put(2, std::string("hello"));
    auto res = lru.get(1);
    if (res) {
    }
}

int main() {
    // test_ring_buffer();
    test_lru_cache();
    return 0;
}