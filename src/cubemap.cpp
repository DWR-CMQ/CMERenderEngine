#include "cubemap.h"

namespace Cme 
{
    void CubemapRenderHelper::multipassDraw(Shader& shader, TextureUniformSource* TextureUniformSource) 
    {
        // Set projection to a 90-degree, 1:1 aspect ratio in order to render a single
        // face of the cube.
        shader.setMat4("projection", glm::perspective(glm::radians(90.0f),
                                                    /*aspect=*/1.0f, 0.1f, 10.0f));

        // TODO: Why are the up vectors negative?
        glm::mat4 faceViews[] =
        {
            glm::lookAt(/*eye=*/glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(/*eye=*/glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(/*eye=*/glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(/*eye=*/glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(/*eye=*/glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(/*eye=*/glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
        };

        for (int cubemapFace = 0; cubemapFace < 6; ++cubemapFace) 
        {
            m_pBuffer->activate(m_iTargetMip, cubemapFace);
            m_pBuffer->clear();

            shader.setMat4("view", faceViews[cubemapFace]);
            m_RoomInstance.draw(shader, TextureUniformSource);
        }

        m_pBuffer->deactivate();
    }

    EquirectCubemap::EquirectCubemap(int width, int height, bool generateMips)
    {
        m_bGenerateMips = generateMips;
        // Optionally allocate memory for mips if requested.

        TextureParams params;
        params.filtering = generateMips ? TextureFiltering::TRILINEAR : TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
        params.generateMips = generateMips ? MipGeneration::ALWAYS : MipGeneration::NEVER;

        ImageSize size;
        size.width = width;
        size.height = height;
        m_spFB = std::make_shared<Framebuffer>(size, BufferType::COLOR_CUBEMAP_HDR, params);
        m_spCubemapRenderHelper = std::make_shared<CubemapRenderHelper>(m_spFB.get());
    }

    void EquirectCubemap::multipassDraw(std::shared_ptr<Texture> spSource)
    {
        // Set up the source.
        spSource->BindToUnit(0, TextureBindType::TEXTURE_2D);
        m_EquirectCubemapShaderInstance.setInt("qrk_equirectMap", 0);

        m_spCubemapRenderHelper->multipassDraw(m_EquirectCubemapShaderInstance);

        if (m_bGenerateMips)
        {
            // Generate mips after having rendered to the cubemap.
            m_spFB->GetTexture()->generateMips();
        }
    }

    unsigned int EquirectCubemap::bindTexture(unsigned int nextTextureUnit,
                                                       Shader& shader) 
    {
        m_spFB->GetTexture()->BindToUnit(nextTextureUnit, TextureBindType::CUBEMAP);
        // Bind sampler uniforms.
        shader.setInt("qrk_cubemap", nextTextureUnit);

        return nextTextureUnit + 1;
    }

}  // namespace Cme
