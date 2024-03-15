#include "cubemap.h"

namespace Cme 
{
    void CubemapRenderHelper::multipassDraw(Shader& shader,
                                            TextureRegistry* textureRegistry) 
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
            m_RoomInstance.draw(shader, textureRegistry);
        }

        m_pBuffer->deactivate();
    }

    EquirectCubemapShader::EquirectCubemapShader()
        : Shader(ShaderPath("assets//shaders//builtin//cubemap.vert"),
                 ShaderPath("assets//shaders//builtin//equirect_cubemap.frag")) {}

    EquirectCubemapConverter::EquirectCubemapConverter(int width, int height,
                                                       bool generateMips)
        : m_BufferInstance(width, height),
        m_CubemapRenderHelperInstance(&m_BufferInstance),
        m_bGenerateMips(generateMips)
    {
        // Optionally allocate memory for mips if requested.

        TextureParams params;
        params.filtering = generateMips ? TextureFiltering::TRILINEAR : TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
        params.generateMips = generateMips ? MipGeneration::ALWAYS : MipGeneration::NEVER;

        m_CubemapInstance = m_BufferInstance.attachTexture(BufferType::COLOR_CUBEMAP_HDR, params);
    }

    void EquirectCubemapConverter::multipassDraw(Texture source)
    {
        // Set up the source.
        source.bindToUnit(0, TextureBindType::TEXTURE_2D);
        m_EquirectCubemapShaderInstance.setInt("qrk_equirectMap", 0);

        m_CubemapRenderHelperInstance.multipassDraw(m_EquirectCubemapShaderInstance);

        if (m_bGenerateMips)
        {
            // Generate mips after having rendered to the cubemap.
            m_CubemapInstance.asTexture().generateMips();
        }
    }

    unsigned int EquirectCubemapConverter::bindTexture(unsigned int nextTextureUnit,
                                                       Shader& shader) 
    {
        m_CubemapInstance.asTexture().bindToUnit(nextTextureUnit, TextureBindType::CUBEMAP);
        // Bind sampler uniforms.
        shader.setInt("qrk_cubemap", nextTextureUnit);

        return nextTextureUnit + 1;
    }

}  // namespace Cme
