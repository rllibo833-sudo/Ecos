#include "ecos_studio/job_system.hpp"

#include <exception>

namespace ecos::studio {

JobSystem::JobSystem(std::size_t worker_count) {
    if (worker_count == 0) {
        worker_count = std::max<std::size_t>(1, std::thread::hardware_concurrency());
        worker_count = std::min<std::size_t>(worker_count, 32);
    }

    workers_.reserve(worker_count);
    try {
        for (std::size_t i = 0; i < worker_count; ++i) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    } catch (...) {
        {
            std::lock_guard lock(mutex_);
            stopping_ = true;
        }
        work_available_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) worker.join();
        }
        throw;
    }
}

JobSystem::~JobSystem() {
    shutdown();
}

void JobSystem::enqueue(std::function<void()> task) {
    {
        std::lock_guard lock(mutex_);
        if (stopping_) throw std::runtime_error("Cannot submit a job after shutdown");
        tasks_.push(std::move(task));
    }
    work_available_.notify_one();
}

void JobSystem::worker_loop() {
    for (;;) {
        std::function<void()> task;
        {
            std::unique_lock lock(mutex_);
            work_available_.wait(lock, [this] { return stopping_ || !tasks_.empty(); });
            if (stopping_ && tasks_.empty()) return;
            task = std::move(tasks_.front());
            tasks_.pop();
            ++active_jobs_;
        }

        try {
            task();
        } catch (...) {
            // Submitted packaged tasks capture exceptions in their futures. This
            // guard keeps the worker alive if a future internal task bypasses that API.
        }

        {
            std::lock_guard lock(mutex_);
            --active_jobs_;
            if (tasks_.empty() && active_jobs_ == 0) idle_.notify_all();
        }
    }
}

void JobSystem::wait_idle() {
    std::unique_lock lock(mutex_);
    idle_.wait(lock, [this] { return tasks_.empty() && active_jobs_ == 0; });
}

void JobSystem::shutdown() {
    {
        std::lock_guard lock(mutex_);
        if (stopping_) return;
        stopping_ = true;
    }
    work_available_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) worker.join();
    }
}

std::size_t JobSystem::worker_count() const noexcept {
    return workers_.size();
}

std::size_t JobSystem::pending_count() const {
    std::lock_guard lock(mutex_);
    return tasks_.size();
}

bool JobSystem::is_stopping() const noexcept {
    std::lock_guard lock(mutex_);
    return stopping_;
}

} // namespace ecos::studio
