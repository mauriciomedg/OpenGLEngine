#pragma once

#include "PipelineParser.h"

#include <functional>
#include <string>
#include <unordered_map>

class Framebuffer;
class FullscreenQuad;
class Material;
class Shader;
class Texture2D;

struct RendererResources
{
    std::unordered_map<std::string, Framebuffer*> renderTargets;
    std::unordered_map<std::string, const Texture2D*> textures;
    std::unordered_map<std::string, Material*> materials;
    std::unordered_map<std::string, Shader*> shaders;
    std::unordered_map<std::string, std::function<void()>> geometryDraws;
    std::unordered_map<std::string, std::function<void(Shader&)>> shaderSetups;

    FullscreenQuad* quad = nullptr;
};

class PipelineExecutor
{
public:
    void executeStage(
        const PipelineStage& stage,
        const RendererResources& resources) const;
};
