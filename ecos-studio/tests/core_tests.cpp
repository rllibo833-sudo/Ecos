#include "ecos_studio/core.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {
int failures = 0;
void check(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void test_runtime_lifecycle() {
    ecos::studio::Runtime runtime;
    check(runtime.state() == ecos::studio::RuntimeState::cold, "runtime begins cold");
    check(!runtime.is_ready(), "cold runtime is not ready");
    check(runtime.initialize(), "runtime initializes from cold");
    check(runtime.is_ready(), "runtime becomes ready");
    check(!runtime.initialize(), "runtime rejects duplicate initialization");
    runtime.shutdown();
    check(runtime.state() == ecos::studio::RuntimeState::stopped, "shutdown stops ready runtime");
    check(!runtime.is_ready(), "stopped runtime is not ready");
    check(!runtime.initialize(), "stopped runtime cannot be initialized again");
    runtime.shutdown();
    check(runtime.state() == ecos::studio::RuntimeState::stopped, "repeated shutdown is safe");
}
void test_logger() {
    std::ostringstream output;
    ecos::studio::Logger logger(output);
    logger.write(ecos::studio::LogLevel::info, "runtime ready");
    logger.write(ecos::studio::LogLevel::error, "example error");
    const auto text = output.str();
    check(text.find("[ECOS][INFO] runtime ready") != std::string::npos, "logger writes level and message");
    check(text.find("[ECOS][ERROR] example error") != std::string::npos, "logger supports error level");
}
}
int main() {
    test_runtime_lifecycle();
    test_logger();
    if (failures != 0) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio core tests passed.\n";
    return 0;
}
