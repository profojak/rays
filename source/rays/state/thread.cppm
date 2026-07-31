module;

#include <atomic>
#include <concepts>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

export module rays:thread;

namespace rays {

/// Pool of worker threads.
export class ThreadPool {
  public:
    /// Create pool sized to hardware.
    ThreadPool() : ThreadPool{DefaultThreadCount()} {}

    /// Create pool with specified number of workers.
    explicit ThreadPool(std::size_t num_threads) { Start(num_threads); }

    /// Signal workers to drain remaining tasks, then join.
    ~ThreadPool() { Stop(); }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    /// Submit callable for execution by any worker thread.
    template <typename F>
        requires std::invocable<F &&>
    void Submit(F &&task) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.emplace(std::forward<F>(task));
            outstanding_.fetch_add(1, std::memory_order_relaxed);
        }
        task_cv_.notify_one();
    }

    /// Return number of worker threads.
    [[nodiscard]] std::size_t Size() const noexcept { return workers_.size(); }

    /// Block until all submitted tasks have completed.
    void WaitIdle() {
        std::size_t current = outstanding_.load(std::memory_order_acquire);
        while (current != 0) {
            outstanding_.wait(current, std::memory_order_acquire);
            current = outstanding_.load(std::memory_order_acquire);
        }
    }

  private:
    /// Return default thread count based on hardware concurrency.
    static std::size_t DefaultThreadCount() noexcept {
        const auto n = std::thread::hardware_concurrency();
        return n == 0 ? 1 : n;
    }

    /// Start thread pool with specified number of workers.
    void Start(std::size_t num_threads) {
        workers_.reserve(num_threads);
        for (std::size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { Worker(); });
        }
    }

    /// Stop thread pool, waiting for all workers to drain their tasks.
    void Stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopping_ = true;
        }
        task_cv_.notify_all();
        workers_.clear();
    }

    /// Worker thread function, processing tasks from the queue.
    void Worker() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                task_cv_.wait(lock,
                              [this] { return stopping_ || !tasks_.empty(); });

                // Drain any pending tasks before exiting on stop.
                if (stopping_ && tasks_.empty()) {
                    return;
                }
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
            if (outstanding_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                outstanding_.notify_all();
            }
        }
    }

    /// Worker threads.
    std::vector<std::jthread> workers_;
    /// Pending tasks.
    std::queue<std::function<void()>> tasks_;
    /// Submitted, but not yet finished tasks.
    std::atomic<std::size_t> outstanding_{0};
    /// Mutex guard.
    std::mutex mutex_;
    /// Signal when work is available or stop is requested.
    std::condition_variable task_cv_;
    /// Flag indicating thread pool is stopping.
    bool stopping_ = false;
};

} // namespace rays
