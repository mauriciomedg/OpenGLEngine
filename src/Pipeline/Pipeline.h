#pragma once

class Framebuffer;
class FullscreenQuad;
class Shader;
class Texture2D;

class Pipeline
{
public:
    void executeCombineStage(
        Framebuffer& combineTarget,
        Shader& combineShader,
        FullscreenQuad& quad,
        const Texture2D& density,
        const Texture2D& noise,
        const Texture2D& metal,
        const Texture2D& lungs,
        const Texture2D& echoMask);
};
