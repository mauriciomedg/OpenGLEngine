#pragma once

#include <string>
#include <vector>

enum class PipelineCommandType
{
    SwitchTarget,
    BindBuffer,
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
