#include "FrameProfiler.h"

#include <cassert>
#include <iomanip>
#include <iostream>

namespace
{
std::size_t stageIndex(ProfileStage stage)
{
    const std::size_t index = static_cast<std::size_t>(stage);
    assert(index < ProfileStageCount);
    return index;
}

double average(double sum, std::size_t count)
{
    return count > 0 ? sum / static_cast<double>(count) : 0.0;
}
}

void FrameProfiler::begin(ProfileStage stage)
{
    timers_[stageIndex(stage)].begin();
}

void FrameProfiler::end(ProfileStage stage)
    noexcept
{
    timers_[stageIndex(stage)].end();
}

void FrameProfiler::beginShadow()
{
    begin(ProfileStage::Shadow);
}

void FrameProfiler::endShadow() noexcept
{
    end(ProfileStage::Shadow);
}

void FrameProfiler::beginGeometry()
{
    begin(ProfileStage::Geometry);
}

void FrameProfiler::endGeometry() noexcept
{
    end(ProfileStage::Geometry);
}

void FrameProfiler::beginSlice()
{
    begin(ProfileStage::Slice);
}

void FrameProfiler::endSlice() noexcept
{
    end(ProfileStage::Slice);
}

void FrameProfiler::beginCombine()
{
    begin(ProfileStage::Combine);
}

void FrameProfiler::endCombine() noexcept
{
    end(ProfileStage::Combine);
}

void FrameProfiler::beginPresent()
{
    begin(ProfileStage::Present);
}

void FrameProfiler::endPresent() noexcept
{
    end(ProfileStage::Present);
}

void FrameProfiler::endFrame(double cpuFrameMs)
{
    cpuFrameSumMs_ += cpuFrameMs;
    ++framesSinceReport_;

    // GPU results are asynchronous and may be reported several CPU frames
    // after the commands were submitted.
    collectGpuSamples();

    if (framesSinceReport_ >= ReportFrameInterval)
    {
        printReport();
        reset();
    }
}

void FrameProfiler::collectGpuSamples()
{
    for (std::size_t i = 0; i < timers_.size(); ++i)
    {
        double milliseconds = 0.0;
        while (timers_[i].tryCollect(milliseconds))
        {
            gpuStats_[i].sumMs += milliseconds;
            ++gpuStats_[i].samples;
        }
    }
}

void FrameProfiler::printReport() const
{
    const auto flags = std::cout.flags();
    const auto precision = std::cout.precision();

    std::cout << std::fixed << std::setprecision(3)
              << "[Profile] frames=" << framesSinceReport_
              << " cpuRender=" << average(cpuFrameSumMs_, static_cast<std::size_t>(framesSinceReport_)) << " ms"
              << " shadow=" << average(gpuStats_[stageIndex(ProfileStage::Shadow)].sumMs, gpuStats_[stageIndex(ProfileStage::Shadow)].samples)
              << " ms(" << gpuStats_[stageIndex(ProfileStage::Shadow)].samples << ")"
              << " geometry=" << average(gpuStats_[stageIndex(ProfileStage::Geometry)].sumMs, gpuStats_[stageIndex(ProfileStage::Geometry)].samples)
              << " ms(" << gpuStats_[stageIndex(ProfileStage::Geometry)].samples << ")"
              << " slice=" << average(gpuStats_[stageIndex(ProfileStage::Slice)].sumMs, gpuStats_[stageIndex(ProfileStage::Slice)].samples)
              << " ms(" << gpuStats_[stageIndex(ProfileStage::Slice)].samples << ")"
              << " combine=" << average(gpuStats_[stageIndex(ProfileStage::Combine)].sumMs, gpuStats_[stageIndex(ProfileStage::Combine)].samples)
              << " ms(" << gpuStats_[stageIndex(ProfileStage::Combine)].samples << ")"
              << " present=" << average(gpuStats_[stageIndex(ProfileStage::Present)].sumMs, gpuStats_[stageIndex(ProfileStage::Present)].samples)
              << " ms(" << gpuStats_[stageIndex(ProfileStage::Present)].samples << ")";

    const std::size_t shadowSkipped = timers_[stageIndex(ProfileStage::Shadow)].skippedCount() - lastReportedSkippedCounts_[stageIndex(ProfileStage::Shadow)];
    const std::size_t geometrySkipped = timers_[stageIndex(ProfileStage::Geometry)].skippedCount() - lastReportedSkippedCounts_[stageIndex(ProfileStage::Geometry)];
    const std::size_t sliceSkipped = timers_[stageIndex(ProfileStage::Slice)].skippedCount() - lastReportedSkippedCounts_[stageIndex(ProfileStage::Slice)];
    const std::size_t combineSkipped = timers_[stageIndex(ProfileStage::Combine)].skippedCount() - lastReportedSkippedCounts_[stageIndex(ProfileStage::Combine)];
    const std::size_t presentSkipped = timers_[stageIndex(ProfileStage::Present)].skippedCount() - lastReportedSkippedCounts_[stageIndex(ProfileStage::Present)];

    if (shadowSkipped != 0)
    {
        std::cout << " shadowSkipped=" << shadowSkipped;
    }

    if (geometrySkipped != 0)
    {
        std::cout << " geometrySkipped=" << geometrySkipped;
    }

    if (sliceSkipped != 0)
    {
        std::cout << " sliceSkipped=" << sliceSkipped;
    }

    if (combineSkipped != 0)
    {
        std::cout << " combineSkipped=" << combineSkipped;
    }

    if (presentSkipped != 0)
    {
        std::cout << " presentSkipped=" << presentSkipped;
    }

    std::cout << '\n';

    std::cout.flags(flags);
    std::cout.precision(precision);
}

void FrameProfiler::reset()
{
    for (std::size_t i = 0; i < timers_.size(); ++i)
    {
        lastReportedSkippedCounts_[i] = timers_[i].skippedCount();
    }

    cpuFrameSumMs_ = 0.0;
    framesSinceReport_ = 0;
    gpuStats_ = {};
}
