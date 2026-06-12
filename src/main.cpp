#include "ring_buffer.hpp"

#include <iostream>

template<class T, size_t SIZE>
void print_state(const RingBuffer<T, SIZE>& rb) {
    using std::cout;
    using std::endl;

    cout << "size: " << rb.size() << endl;
    cout << "full: " << rb.full() << endl;
    cout << "empty: " << rb.empty() << endl;
    cout << endl;
}

int main() {
    auto rb = RingBuffer<int, 5>{};
    print_state(rb);
    for (int i = 0; i<10; i++) {
        rb.push(i);
        print_state(rb);
    }

    for (int i = 0; i<6; i++) {
        if (const auto val = rb.pop()) {
            std::cout << "val: " << *val << std::endl;
        }
    }
    
    return 0;
}