#pragma once

#include <glad/glad.h>

#include <array>
#include <cstddef>

class GpuTimer
{
public:
    static constexpr std::size_t QueryCount = 8;

    GpuTimer();
    ~GpuTimer();

    GpuTimer(const GpuTimer&) = delete;
    GpuTimer& operator=(const GpuTimer&) = delete;

    void begin();
    void end() noexcept;

    bool tryCollect(double& milliseconds);
    std::size_t skippedCount() const noexcept;

private:
    // GL_TIME_ELAPSED queries for one context cannot be nested; render passes
    // should be timed sequentially.
    std::array<GLuint, QueryCount> queries_{};

    std::size_t writeIndex_ = 0;
    std::size_t readIndex_ = 0;
    std::size_t pendingCount_ = 0;
    std::size_t skippedCount_ = 0;

    bool active_ = false;
    bool skipped_ = false;
};
