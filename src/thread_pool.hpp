#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <ranges>
#include <thread>
#include <vector>

class ThreadPool {
  public:
    explicit ThreadPool(size_t thread_count) : m_stop(false) {
        for (size_t i = 0; i < thread_count; i++) {
            m_threads.emplace_back([this] {
                while (true) {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_cv.wait(lock, [this] { return m_stop || !m_queue.empty(); });
                    if (m_stop && m_queue.empty()) {
                        return;
                    }
                    auto task = std::move(m_queue.front());
                    m_queue.pop();
                    lock.unlock();
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stop = true;
        }
        m_cv.notify_all();
        for (auto& t : m_threads) {
            t.join();
        }
    }

    template <typename Task> void enqueue(Task&& task) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.emplace(std::forward<Task>(task));
        }
        m_cv.notify_one();
    }

  private:
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_stop;
};

// Main components:
// - worker threads: wait for work -> execute work -> repeat
// - A place to store pending work: Queue as a container seems fitting
//    - multiple threads might submit work simultaneously
//    - multiple workser may attempt to take work simultaneously
//    - tasks should generally be processed in the order they were submitted
// - A synchronization mechanism: mutex/condition var
//    - workers need a way to sleep while there is no work, and wake when new arrives ( no continuous checking)
// - Shutdown state
//   - workers must be informed that no more work will arrive, and should eventually stop
//      - finish all queued work or
//      - discard remaining work and stop immediatly
