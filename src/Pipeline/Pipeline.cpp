#include "Pipeline.h"

#include "../FullscreenQuad.h"
#include "../Renderer/Framebuffer.h"
#include "../Renderer/Texture2D.h"
#include "../Shader.h"

#include <glad/glad.h>

void Pipeline::executeCombineStage(
    Framebuffer& combineTarget,
    Shader& combineShader,
    FullscreenQuad& quad,
    const Texture2D& density,
    const Texture2D& noise,
    const Texture2D& metal,
    const Texture2D& lungs,
    const Texture2D& echoMask)
{
    // RenderTarget = Framebuffer, SwitchTarget = Framebuffer::bind.
    combineTarget.bind();

    glClearColor(0.02f, 0.03f, 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    combineShader.use();

    // BindBuffer = sampler uniform assignment plus Texture2D::bind.
    combineShader.setInt("buf0", 0);
    combineShader.setInt("buf1", 1);
    combineShader.setInt("buf2", 2);
    combineShader.setInt("buf3", 3);
    combineShader.setInt("echoMask", 4);

    density.bind(0);
    noise.bind(1);
    metal.bind(2);
    lungs.bind(3);
    echoMask.bind(4);

    // DrawQuad = FullscreenQuad::draw.
    quad.draw();

    // UnbindBuffers = Texture2D::unbind.
    Texture2D::unbind(0);
    Texture2D::unbind(1);
    Texture2D::unbind(2);
    Texture2D::unbind(3);
    Texture2D::unbind(4);
    glActiveTexture(GL_TEXTURE0);

}
