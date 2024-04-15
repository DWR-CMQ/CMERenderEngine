#include "deferred.h"

namespace Cme
{
    DeferredGeometryPassShader::DeferredGeometryPassShader()
        : Shader(ShaderPath("assets//shaders//builtin//deferred.vert"),
                 ShaderPath("assets//shaders//builtin//deferred.frag")) {}

    // GBuffer�����ж������(֡��������ж������)
    // GBuffer�;����˲�������֡����Ĺ��캯������Ӷ����ɫ���� ��Ϊ���кܶ�����֡������ֻ��һ����ɫ����
    GBuffer::GBuffer(int width, int height) : Framebuffer(width, height) 
    {
        // Need to use a zero clear color, or else the G-Buffer won't work properly.
        setClearColor(glm::vec4(0.0f));
        // Create and attach all components of the G-Buffer.
        // Don't need to read from depth, so we attach as a renderbuffer.
        attachRenderbuffer(Cme::BufferType::DEPTH_AND_STENCIL);                     // ��Ⱦ������󸽼�
        // Position and normal are stored as "HDR" colors for higher precision.
        // TODO: Positions can be un-projected from depth without storing them.

        // RGB used for position, alpha used for AO. 
        AttachTexture2FB(Cme::BufferType::COLOR_HDR_ALPHA);     
        // RGB used for normal, alpha used for roughness.
        AttachTexture2FB(Cme::BufferType::COLOR_SNORM_ALPHA); 
        // RGB used for albedo, alpha used for metallic.
        AttachTexture2FB(Cme::BufferType::COLOR_ALPHA); 
        // RGB used for emission color, alpha channel unused.
        AttachTexture2FB(Cme::BufferType::COLOR_ALPHA); 
    }

    unsigned int GBuffer::bindTexture(unsigned int nextTextureUnit, Shader& shader) 
    {
        GetTexture(1)->BindToUnit(nextTextureUnit + 0);
        GetTexture(2)->BindToUnit(nextTextureUnit + 1);
        GetTexture(3)->BindToUnit(nextTextureUnit + 2);
        GetTexture(4)->BindToUnit(nextTextureUnit + 3);
        // Bind sampler uniforms.
        shader.setInt("gPositionAO", nextTextureUnit + 0);
        shader.setInt("gNormalRoughness", nextTextureUnit + 1);
        shader.setInt("gAlbedoMetallic", nextTextureUnit + 2);
        shader.setInt("gEmission", nextTextureUnit + 3);

        return nextTextureUnit + 4;
    }

}  // namespace Cme
