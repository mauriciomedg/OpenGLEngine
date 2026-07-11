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

class FrameProfiler
{
public:
    FrameProfiler() = default;

    FrameProfiler(const FrameProfiler&) = delete;
    FrameProfiler& operator=(const FrameProfiler&) = delete;

    void begin(ProfileStage stage);
    void end(ProfileStage stage);

    void beginShadow();
    void endShadow();
    void beginGeometry();
    void endGeometry();
    void beginSlice();
    void endSlice();
    void beginCombine();
    void endCombine();
    void beginPresent();
    void endPresent();

    void endFrame(double cpuFrameMs);

private:
    struct StageStats
    {
        double sumMs = 0.0;
        std::size_t samples = 0;
    };

    static constexpr std::size_t StageCount = static_cast<std::size_t>(ProfileStage::Count);
    static constexpr int ReportFrameInterval = 120;

    void collectGpuSamples();
    void printReport() const;
    void reset();

    std::array<GpuTimer, StageCount> timers_;
    std::array<StageStats, StageCount> gpuStats_{};

    double cpuFrameSumMs_ = 0.0;
    int framesSinceReport_ = 0;
};
