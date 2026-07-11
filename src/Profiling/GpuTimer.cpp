#include "GpuTimer.h"

#include <cassert>
#include <stdexcept>

GpuTimer::GpuTimer()
{
    glGenQueries(static_cast<GLsizei>(queries_.size()), queries_.data());
}

GpuTimer::~GpuTimer()
{
    assert(!active_);
    glDeleteQueries(static_cast<GLsizei>(queries_.size()), queries_.data());
}

void GpuTimer::begin()
{
    if (active_)
    {
        throw std::logic_error("GPU timer query is already active.");
    }

    if (pendingCount_ == QueryCount)
    {
        skipped_ = true;
        return;
    }

    glBeginQuery(GL_TIME_ELAPSED, queries_[writeIndex_]);
    active_ = true;
}

void GpuTimer::end()
{
    if (skipped_)
    {
        skipped_ = false;
        return;
    }

    if (!active_)
    {
        throw std::logic_error("GPU timer query is not active.");
    }

    glEndQuery(GL_TIME_ELAPSED);
    writeIndex_ = (writeIndex_ + 1) % QueryCount;
    ++pendingCount_;
    active_ = false;
}

bool GpuTimer::tryCollect(double& milliseconds)
{
    if (pendingCount_ == 0)
    {
        return false;
    }

    const GLuint query = queries_[readIndex_];

    GLint available = 0;
    glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
    if (available == GL_FALSE)
    {
        return false;
    }

    GLuint64 elapsedNanoseconds = 0;
    glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsedNanoseconds);
    milliseconds = static_cast<double>(elapsedNanoseconds) / 1000000.0;

    readIndex_ = (readIndex_ + 1) % QueryCount;
    --pendingCount_;
    return true;
}
