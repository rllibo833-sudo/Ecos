#pragma once

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace ecos::studio {

// Native worker pool for engine systems. Shutdown drains queued work; exceptions
// from submitted jobs are delivered through their futures.
class JobSystem {
public:
    explicit JobSystem(std::size_t worker_count = 0);
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;
    JobSystem(JobSystem&&) = delete;
    JobSystem& operator=(JobSystem&&) = delete;

    template <typename F, typename... Args>
    auto submit(F&& function, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>> {
        using Return = std::invoke_result_t<F, Args...>;
        auto callable = std::forward<F>(function);
        auto arguments = std::make_tuple(std::forward<Args>(args)...);
        auto packaged = std::make_shared<std::packaged_task<Return()>>(
            [callable = std::move(callable), arguments = std::move(arguments)]() mutable -> Return {
                return std::apply(
                    [&callable](auto&&... unpacked) -> Return {
                        return std::invoke(std::move(callable),
                            std::forward<decltype(unpacked)>(unpacked)...);
                    },
                    std::move(arguments));
            });
        auto future = packaged->get_future();
        enqueue([packaged] { (*packaged)(); });
        return future;
    }

    void wait_idle();
    void shutdown();
    [[nodiscard]] std::size_t worker_count() const noexcept;
    [[nodiscard]] std::size_t pending_count() const;
    [[nodiscard]] bool is_stopping() const noexcept;

private:
    void enqueue(std::function<void()> task);
    void worker_loop();

    mutable std::mutex mutex_;
    std::condition_variable work_available_;
    std::condition_variable idle_;
    std::queue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
    std::size_t active_jobs_{0};
    bool stopping_{false};
};

} // namespace ecos::studio
