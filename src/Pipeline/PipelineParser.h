#pragma once

#include <string>
#include <vector>

enum class PipelineCommandType
{
    SwitchTarget,
    ClearTarget,
    BindBuffer,
    DrawGeometry,
    DrawQuad,
    UnbindBuffers
};

struct PipelineCommand
{
    PipelineCommandType type;
    std::string target;
    std::string sampler;
    std::string sourceRT;
    int bufIndex = 0;
    std::string material;
    std::string context;
    bool clearDepth = false;
    bool clearColor0 = false;
    float clearR = 0.0f;
    float clearG = 0.0f;
    float clearB = 0.0f;
    float clearA = 1.0f;
    float clearDepthValue = 1.0f;
    std::string cullFrustum;
};

struct PipelineStage
{
    std::string id;
    bool enabled = true;
    std::vector<PipelineCommand> commands;
};

class PipelineParser
{
public:
    std::vector<PipelineStage> load(const std::string& path) const;
};
