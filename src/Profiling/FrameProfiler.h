#pragma once

#include "GpuTimer.h"

#include <array>
#include <cstddef>

struct FrameProfile
{
    double cpuFrameMs = 0.0;
    double shadowGpuMs = 0.0;
    double geometryGpuMs = 0.0;
    double sliceGpuMs = 0.0;
    double combineGpuMs = 0.0;
    double presentGpuMs = 0.0;
};

enum class ProfileStage : std::size_t
{
    Shadow = 0,
    Geometry,
    Slice,
    Combine,
    Present,
    Count
};

static constexpr std::size_t ProfileStageCount =
    static_cast<std::size_t>(ProfileStage::Count);

class FrameProfiler
{
public:
    FrameProfiler() = default;

    FrameProfiler(const FrameProfiler&) = delete;
    FrameProfiler& operator=(const FrameProfiler&) = delete;

    void begin(ProfileStage stage);
    void end(ProfileStage stage) noexcept;

    void beginShadow();
    void endShadow() noexcept;
    void beginGeometry();
    void endGeometry() noexcept;
    void beginSlice();
    void endSlice() noexcept;
    void beginCombine();
    void endCombine() noexcept;
    void beginPresent();
    void endPresent() noexcept;

    void endFrame(double cpuFrameMs);

private:
    struct StageStats
    {
        double sumMs = 0.0;
        std::size_t samples = 0;
    };

    static constexpr int ReportFrameInterval = 120;

    void collectGpuSamples();
    void printReport() const;
    void reset();

    std::array<GpuTimer, ProfileStageCount> timers_;
    std::array<StageStats, ProfileStageCount> gpuStats_{};
    std::array<std::size_t, ProfileStageCount> lastReportedSkippedCounts_{};

    double cpuFrameSumMs_ = 0.0;
    int framesSinceReport_ = 0;
};
