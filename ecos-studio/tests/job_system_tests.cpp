#include "ecos_studio/job_system.hpp"

#include <atomic>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
int failures = 0;
template <typename T>
void check(const T& condition, const char* message) {
    if (!static_cast<bool>(condition)) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void test_job_execution_and_idle() {
    using namespace ecos::studio;
    JobSystem jobs(3);
    check(jobs.worker_count() == 3, "uses the requested worker count");

    std::atomic<int> completed{0};
    std::vector<std::future<int>> futures;
    futures.reserve(120);
    for (int i = 0; i < 120; ++i) {
        futures.push_back(jobs.submit([&completed, i] {
            completed.fetch_add(1, std::memory_order_relaxed);
            return i * i;
        }));
    }

    long long sum = 0;
    for (int i = 0; i < static_cast<int>(futures.size()); ++i) {
        sum += futures[static_cast<std::size_t>(i)].get();
    }
    jobs.wait_idle();
    check(completed.load(std::memory_order_relaxed) == 120, "executes every submitted job once");
    check(sum == 568820, "returns each job result through its future");
    check(jobs.pending_count() == 0, "queue is empty after wait_idle");
}

void test_exception_and_shutdown() {
    using namespace ecos::studio;
    JobSystem jobs(2);
    auto failure = jobs.submit([]() -> int { throw std::runtime_error("job failure"); });
    bool exception_delivered = false;
    try {
        (void)failure.get();
    } catch (const std::runtime_error& error) {
        exception_delivered = std::string(error.what()) == "job failure";
    }
    check(exception_delivered, "delivers job exceptions through futures");

    jobs.shutdown();
    check(jobs.is_stopping(), "reports shutdown state");
    bool submit_rejected = false;
    try {
        (void)jobs.submit([] { return 1; });
    } catch (const std::runtime_error&) {
        submit_rejected = true;
    }
    check(submit_rejected, "rejects submissions after shutdown");
}
}

int main() {
    test_job_execution_and_idle();
    test_exception_and_shutdown();
    if (failures != 0) {
        std::cerr << failures << " job-system test(s) failed.\n";
        return 1;
    }
    std::cout << "ECOS Studio job system tests passed.\n";
    return 0;
}
