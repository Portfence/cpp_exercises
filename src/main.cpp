#include "echo.hpp"
#include "lru_cache.hpp"
#include "ring_buffer.hpp"
#include "smart_ptr.hpp"
#include "thread_pool.hpp"
#include "tracker/tracker.hpp"
#include <iostream>
#include <print>
#include <string>

template <class T, size_t SIZE> void print_state(const AtomicRingBuffer<T, SIZE>& rb) {
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

void test_shared_ptr() {
    struct A {
        int i = 0;
    };

    // Move constructor
    auto a = SharedPtr(new int(1));
    // Move constructor
    auto b = SharedPtr(new int(2));
    // copy constructor
    auto c = a;
    // copy assignment
    c = b;
    // self copy (guarded)
    c = c;
    auto d = SharedPtr(new A{.i = 1});
    auto e = SharedPtr(new A{.i = 2});

    d = std::move(e);

    std::cout << "val: " << *a << std::endl;
    std::cout << "val: " << *b << std::endl;
    std::cout << "val: " << d->i << std::endl;
}

void test_thread_pool() {
    auto thread_pool = ThreadPool(9);

    for (int i = 0; i < 10; i++) {
        thread_pool.enqueue([i] {
            using namespace std::chrono_literals;
            int j = 0;
            while (j < 5) {
                std::println("task {} working on step {}..", i, j);
                j++;
                std::this_thread::sleep_for(1s);
            }
        });
    }
    std::println("enqueued 10 tasks");
}

using std::println;

int main() {
    println("running Exercices bin");
    // test_ring_buffer();
    // test_lru_cache();
    // test_shared_ptr();
    // test_thread_pool();
    // coroutines::run_exercise();
    // if (auto result = echo::run_exercise(); !result.has_value()) {
    //     println("error: {}", result.error());
    // }

    auto tracker = tracker::Tracker();
    return tracker.run();


}
