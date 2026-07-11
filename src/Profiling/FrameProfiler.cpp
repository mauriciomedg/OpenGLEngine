#include "FrameProfiler.h"

#include <iomanip>
#include <iostream>

namespace
{
std::size_t stageIndex(ProfileStage stage)
{
    return static_cast<std::size_t>(stage);
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
{
    timers_[stageIndex(stage)].end();
}

void FrameProfiler::beginShadow()
{
    begin(ProfileStage::Shadow);
}

void FrameProfiler::endShadow()
{
    end(ProfileStage::Shadow);
}

void FrameProfiler::beginGeometry()
{
    begin(ProfileStage::Geometry);
}

void FrameProfiler::endGeometry()
{
    end(ProfileStage::Geometry);
}

void FrameProfiler::beginSlice()
{
    begin(ProfileStage::Slice);
}

void FrameProfiler::endSlice()
{
    end(ProfileStage::Slice);
}

void FrameProfiler::beginCombine()
{
    begin(ProfileStage::Combine);
}

void FrameProfiler::endCombine()
{
    end(ProfileStage::Combine);
}

void FrameProfiler::beginPresent()
{
    begin(ProfileStage::Present);
}

void FrameProfiler::endPresent()
{
    end(ProfileStage::Present);
}

void FrameProfiler::endFrame(double cpuFrameMs)
{
    cpuFrameSumMs_ += cpuFrameMs;
    ++framesSinceReport_;

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
              << " cpu=" << average(cpuFrameSumMs_, static_cast<std::size_t>(framesSinceReport_)) << " ms"
              << " shadow=" << average(gpuStats_[stageIndex(ProfileStage::Shadow)].sumMs, gpuStats_[stageIndex(ProfileStage::Shadow)].samples) << " ms"
              << " geometry=" << average(gpuStats_[stageIndex(ProfileStage::Geometry)].sumMs, gpuStats_[stageIndex(ProfileStage::Geometry)].samples) << " ms"
              << " slice=" << average(gpuStats_[stageIndex(ProfileStage::Slice)].sumMs, gpuStats_[stageIndex(ProfileStage::Slice)].samples) << " ms"
              << " combine=" << average(gpuStats_[stageIndex(ProfileStage::Combine)].sumMs, gpuStats_[stageIndex(ProfileStage::Combine)].samples) << " ms"
              << " present=" << average(gpuStats_[stageIndex(ProfileStage::Present)].sumMs, gpuStats_[stageIndex(ProfileStage::Present)].samples) << " ms\n";

    std::cout.flags(flags);
    std::cout.precision(precision);
}

void FrameProfiler::reset()
{
    cpuFrameSumMs_ = 0.0;
    framesSinceReport_ = 0;
    gpuStats_ = {};
}
