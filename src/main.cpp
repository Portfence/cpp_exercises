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
    rb.push(1);
    print_state(rb);
    rb.push(2);
    print_state(rb);
    rb.push(3);
    print_state(rb); 
    rb.push(4);
    print_state(rb);
    rb.push(4);
    print_state(rb);
    rb.push(4);
    print_state(rb);
    rb.push(4);
    print_state(rb);
    rb.pop();
    print_state(rb);
    rb.pop();
    print_state(rb);
    rb.pop();
    print_state(rb);
    rb.pop();
    print_state(rb);
    rb.pop();
    print_state(rb);
    rb.pop();
    print_state(rb);
    rb.pop();
    print_state(rb);
    rb.pop();
    print_state(rb); 
    return 0;
}