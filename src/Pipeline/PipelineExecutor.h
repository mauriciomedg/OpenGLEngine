#pragma once

#include "PipelineParser.h"

#include <string>
#include <unordered_map>

class Framebuffer;
class FullscreenQuad;
class Shader;
class Texture2D;

struct PipelineResources
{
    std::unordered_map<std::string, Framebuffer*> renderTargets;
    std::unordered_map<std::string, const Texture2D*> textures;
    std::unordered_map<std::string, Shader*> shaders;

    FullscreenQuad* quad = nullptr;
};

class PipelineExecutor
{
public:
    void executeStage(
        const PipelineStage& stage,
        const PipelineResources& resources) const;
};
