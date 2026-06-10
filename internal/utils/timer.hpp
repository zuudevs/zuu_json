/**
 * @file timer.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-10
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include <chrono>

namespace zuu::utils {

class Timer {
  public:
    using TimeT = std::chrono::steady_clock::time_point;

    Timer() noexcept = default;
    Timer(Timer&&) noexcept = default;
    ~Timer() noexcept = default;
    Timer& operator=(Timer&&) noexcept = default;
    Timer(const Timer&) noexcept = delete;
    Timer& operator=(const Timer&) noexcept = delete;

    void start() noexcept {
        now(start_);
    }

    [[nodiscard]] auto duration() noexcept {
        now(end_);
        auto elapsed_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_);
        reset(start_);
        reset(end_);
        start();
        return elapsed_time_ns;
    }

  private:
    TimeT start_;
    TimeT end_;

    void now(TimeT& time) noexcept {
        time = std::chrono::steady_clock::now();
    }

    void reset(TimeT& time) noexcept {
        time = TimeT{};
    }
};

} // namespace zuu::utils