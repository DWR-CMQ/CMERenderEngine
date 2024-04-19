#include <glad/glad.h>
#include "framebuffer.h"
#include "common_helper.h"

namespace Cme 
{
    // ����֡������ӳ���Ⱦͬʱ���� ���Ա�ҪҪ��һ���ṹ����ͬʱ�������Ⱦ�������洢����
    // ������ת��ΪTexture �����ǽ�ID(���)���ݸ�Texture
    std::shared_ptr<Texture> Attachment::Transform2Texture()
    {
        // ���ж�ר���������Ⱦ������󸽼� Ҳ����˵��Ⱦ����Ŷ������ת��ΪTexture
        // ��ô���仰˵ �����Ⱦ������󸽼������浽m_vecAttachments�о�OK 
        // ��Ҳ���� ѭ����ʱ�� ��Ⱦ�����������Ҫ�� ����Ҫpush��m_vecAttachments��
        // ������������ m_vecAttachments������Ҫ�����
        if (m_eTarget != AttachmentTarget::TEXTURE)
        {
            throw FramebufferException("ERROR::FRAMEBUFFER::INVALID_ATTACHMENT_TARGET");
        }
        //Texture texture;
        auto spTexture = std::make_shared<Texture>();
        spTexture->m_uiID = m_uiID;
        spTexture->m_eType = m_eTextureType;
        spTexture->m_iWidth = m_iWidth;
        spTexture->m_iHeight = m_iHeight;
        spTexture->m_iNumMips = m_iNumMips;
        return spTexture;
    }

    Framebuffer::Framebuffer(int width, int height, int samples)
        : m_iWidth(width), m_iHeight(height), m_iSamples(samples)
    {
        glGenFramebuffers(1, &fbo_);
    }

    Framebuffer::Framebuffer(ImageSize size, BufferType type, TextureParams params, int samples)
    {
        m_iWidth = size.width;
        m_iHeight = size.height;
        m_iSamples = samples;

        glGenFramebuffers(1, &fbo_);

        checkFlags(type);
        activate();

        TextureType textureType = TextureType::TEXTURE_2D;
        GLenum textureTarget = m_iSamples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        // Special case cubemaps.
        if (type == BufferType::COLOR_CUBEMAP_HDR || type == BufferType::COLOR_CUBEMAP_HDR_ALPHA)
        {
            textureTarget = GL_TEXTURE_CUBE_MAP;
            textureType = TextureType::CUBEMAP;
        }

        GLenum internalFormat = bufferTypeToGlInternalFormat(type);
        auto spTexture = std::make_shared<Texture>();
        spTexture->Create(m_iWidth, m_iHeight, internalFormat, params, type);
        spTexture->SetTextureType(textureType);

        // Attach the texture to the framebuffer.
        int colorAttachmentIndex = m_iNumColorAttachments;

        GLenum attachmentType = bufferTypeToGlAttachmentType(type, colorAttachmentIndex);
        GLenum textarget = textureType == TextureType::CUBEMAP ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : textureTarget;
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, textarget, spTexture->getId(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            throw FramebufferException("ERROR::FRAMEBUFFER::TEXTURE::INCOMPLETE");
        }

        updateFlags(type);
        updateBufferSources();

        glBindTexture(textureTarget, 0);
        deactivate();
       
        saveAttachment(spTexture->getId(), spTexture->getNumMips(), AttachmentTarget::TEXTURE, type, colorAttachmentIndex, textureType);
    }

    Framebuffer::~Framebuffer() 
    {
        glDeleteFramebuffers(1, &fbo_);
        // TODO: Delete attachments.
    }

    void Framebuffer::activate(int mipLevel, int cubemapFace) 
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

        // Activate the specified mip level (usually 0).
        for (Attachment& attachment : m_vecAttachments)
        {
            GLenum attachmentType = bufferTypeToGlAttachmentType(attachment.m_eType, attachment.m_iColorAttachmentIndex);

            switch (attachment.m_eTarget)
            {
                case AttachmentTarget::TEXTURE:
                {
                    GLenum target = GL_TEXTURE_2D;
                    if (cubemapFace >= 0) 
                    {
                        if (cubemapFace >= 6) 
                        {
                            throw FramebufferException(
                                "ERROR::FRAMEBUFFER::CUBEMAP_FACE_OUT_OF_RANGE");
                        }
                        target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubemapFace;
                    }
                    glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, target, attachment.m_uiID, mipLevel);
                } break;
                case AttachmentTarget::RENDERBUFFER:
                    // Perform some checks.
                    if (mipLevel != 0) 
                    {
                        // Non-0 mips are only allowed for textures.
                        throw FramebufferException(
                            "ERROR::FRAMEBUFFER::MIP_ACTIVATED_FOR_RENDERBUFFER");
                    }
                    if (cubemapFace >= 0) 
                    {
                        // Renderbuffers currently can't be cubemaps.
                        throw FramebufferException(
                            "ERROR::FRAMEBUFFER::CUBEMAP_FACE_GIVEN_FOR_RENDERBUFFER");
                    }
                    break;
            }
        }

        ImageSize mipSize = CommonHelper::calculateMipLevel(m_iWidth, m_iHeight, mipLevel);
        glViewport(0, 0, mipSize.width, mipSize.height);
    }

    void Framebuffer::deactivate() 
    { 
        // unbind fbo ���fbo
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    }

    ImageSize Framebuffer::getSize() 
    {
        ImageSize size;
        size.width = m_iWidth;
        size.height = m_iHeight;
        return size;
    }

    /// ������ô����֡�����д���
    /// @brief �������� ��ӵ�֡������ 
    /// @param type 
    /// @return ����
    Attachment Framebuffer::AttachTexture2FB(BufferType type)
    {
        TextureParams params;
        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
        return AttachTexture2FB_i(type, params);
    }

    /// @brief �������� �����ӵ�֡������
    /// @param type 
    /// @param params 
    /// @return 
    Attachment Framebuffer::AttachTexture2FB_i(BufferType type, const TextureParams& params)
    {
        checkFlags(type);
        activate();

        TextureType textureType = TextureType::TEXTURE_2D;
        GLenum textureTarget = m_iSamples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        // Special case cubemaps.
        if (type == BufferType::COLOR_CUBEMAP_HDR || type == BufferType::COLOR_CUBEMAP_HDR_ALPHA) 
        {
            textureTarget = GL_TEXTURE_CUBE_MAP;
            textureType = TextureType::CUBEMAP;
        }

        GLenum internalFormat = bufferTypeToGlInternalFormat(type);
        auto spTexture = std::make_shared<Texture>();
        spTexture->Create(m_iWidth, m_iHeight, internalFormat, params, type);

        // Attach the texture to the framebuffer.
        int colorAttachmentIndex = m_iNumColorAttachments;

        GLenum attachmentType = bufferTypeToGlAttachmentType(type, colorAttachmentIndex);
        GLenum textarget = textureType == TextureType::CUBEMAP ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : textureTarget;
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, textarget, spTexture->getId(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
        {
            throw FramebufferException("ERROR::FRAMEBUFFER::TEXTURE::INCOMPLETE");
        }

        updateFlags(type);
        updateBufferSources();

        glBindTexture(textureTarget, 0);
        deactivate();

        return saveAttachment(spTexture->getId(), spTexture->getNumMips(), AttachmentTarget::TEXTURE, type, colorAttachmentIndex, textureType);
    }

    Attachment Framebuffer::attachRenderbuffer(BufferType type)
    {
        checkFlags(type);
        activate();

        // Create and configure renderbuffer.
        // Renderbuffers are similar to textures, but they generally cannot be read
        // from easily. In exchange, their render data is stored in a native format,
        // so they are perfect for use cases that require writing (such as the final
        // frame, or depth/stencil attachments).
        // TODO: Pull out into a renderbuffer class?
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);

        GLenum internalFormat = bufferTypeToGlInternalFormat(type);
        if (m_iSamples)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_iSamples, internalFormat, m_iWidth, m_iHeight);
        } 
        else 
        {
            glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, m_iWidth, m_iHeight);
        }

        // Attach the renderbuffer to the framebuffer.
        int colorAttachmentIndex = m_iNumColorAttachments;
        GLenum attachmentType = bufferTypeToGlAttachmentType(type, colorAttachmentIndex);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            throw FramebufferException("ERROR::FRAMEBUFFER::RENDERBUFFER::INCOMPLETE");
        }

        updateFlags(type);
        updateBufferSources();

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        deactivate();

        return saveAttachment(rbo, 1, AttachmentTarget::RENDERBUFFER, type, colorAttachmentIndex, TextureType::TEXTURE_2D);
    }

    Attachment Framebuffer::getTexture(BufferType type)
    {
        return getAttachment(AttachmentTarget::TEXTURE, type);
    }

    Attachment Framebuffer::getRenderbuffer(BufferType type)
    {
        return getAttachment(AttachmentTarget::RENDERBUFFER, type);
    }

    void Framebuffer::blit(Framebuffer& target, GLenum bits)
    {
        // TODO: This doesn't handle non-mip0 blits.
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.fbo_);
        glBlitFramebuffer(0, 0, m_iWidth, m_iHeight, 0, 0, m_iWidth, m_iHeight, bits,
                        GL_NEAREST);
        deactivate();
    }

    // defer shaderר�� 
    void Framebuffer::blitToDefault(GLenum bits)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, m_iWidth, m_iHeight, 0, 0, m_iWidth, m_iHeight, bits, GL_NEAREST);
        deactivate();
    }

    Attachment Framebuffer::saveAttachment(unsigned int id, int numMips,
                                           AttachmentTarget target, 
                                           BufferType type,
                                           int colorAttachmentIndex,
                                           TextureType textureType)
    {
        Attachment attachment;
        attachment.m_uiID = id;
        attachment.m_iWidth = m_iWidth;
        attachment.m_iHeight = m_iHeight;
        attachment.m_iNumMips = numMips;
        attachment.m_eTarget = target;
        attachment.m_eType = type;
        attachment.m_iColorAttachmentIndex = colorAttachmentIndex;
        attachment.m_eTextureType = textureType;

        m_vecAttachments.push_back(attachment);
        return attachment;
    }

    Attachment Framebuffer::getAttachment(AttachmentTarget target,
                                          BufferType type) 
    {
        for (Attachment& attachment : m_vecAttachments)
        {
            if (attachment.m_eTarget == target && attachment.m_eType == type)
            {
                return attachment;
            }
        }
        throw FramebufferException("ERROR::FRAMEBUFFER::ATTACHMENT_NOT_FOUND");
    }

    void Framebuffer::checkFlags(BufferType type)
    {
        switch (type) 
        {
            case BufferType::COLOR:
            case BufferType::COLOR_HDR:
            case BufferType::COLOR_SNORM:
            case BufferType::COLOR_CUBEMAP_HDR:
            case BufferType::COLOR_ALPHA:
            case BufferType::COLOR_HDR_ALPHA:
            case BufferType::COLOR_SNORM_ALPHA:
            case BufferType::COLOR_CUBEMAP_HDR_ALPHA:
            case BufferType::GRAYSCALE:
                // Multiple color attachments OK.
                return;
            case BufferType::DEPTH:
                if (m_hasDepthAttachment)
                {
                    throw FramebufferException(
                        "ERROR::FRAMEBUFFER::BUFFER_TYPE_ALREADY_IN_USE\n" +
                        std::to_string(static_cast<int>(type)));
                }
                return;
            case BufferType::STENCIL:
                if (m_hasStencilAttachment)
                {
                    throw FramebufferException(
                        "ERROR::FRAMEBUFFER::BUFFER_TYPE_ALREADY_IN_USE\n" +
                        std::to_string(static_cast<int>(type)));
                }
                return;
            case BufferType::DEPTH_AND_STENCIL:
                if (m_hasDepthAttachment || m_hasStencilAttachment)
                {
                    throw FramebufferException(
                        "ERROR::FRAMEBUFFER::BUFFER_TYPE_ALREADY_IN_USE\n" +
                        std::to_string(static_cast<int>(type)));
                }
                return;
        }
    }

    void Framebuffer::updateFlags(BufferType type) 
    {
        switch (type) 
        {
            case BufferType::COLOR:
            case BufferType::COLOR_HDR:
            case BufferType::COLOR_SNORM:
            case BufferType::COLOR_CUBEMAP_HDR:
            case BufferType::COLOR_ALPHA:
            case BufferType::COLOR_HDR_ALPHA:
            case BufferType::COLOR_SNORM_ALPHA:
            case BufferType::COLOR_CUBEMAP_HDR_ALPHA:
            case BufferType::GRAYSCALE:
                m_hasColorAttachment = true;
                m_iNumColorAttachments++;
                return;
            case BufferType::DEPTH:
                m_hasDepthAttachment = true;
                return;
            case BufferType::STENCIL:
                m_hasStencilAttachment = true;
                return;
            case BufferType::DEPTH_AND_STENCIL:
                m_hasDepthAttachment = true;
                m_hasStencilAttachment = true;
                return;
        }
        throw FramebufferException("ERROR::FRAMEBUFFER::INVALID_BUFFER_TYPE\n" +
                                    std::to_string(static_cast<int>(type)));
    }

    void Framebuffer::updateBufferSources() 
    {
        if (m_hasColorAttachment)
        {
            if (m_iNumColorAttachments == 1)
            {
                glDrawBuffer(GL_COLOR_ATTACHMENT0);
            } 
            else
            {
                std::vector<unsigned int> attachments;
                for (int i = 0; i < m_iNumColorAttachments; i++)
                {
                    attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
                }
                glDrawBuffers(m_iNumColorAttachments, attachments.data());
            }
            // Always read from attachment 0.
            glReadBuffer(GL_COLOR_ATTACHMENT0);
        } 
        else 
        {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
    }

    void Framebuffer::clear() 
    {
        glClearColor(m_vec4ClearColor.r, m_vec4ClearColor.g, m_vec4ClearColor.b, m_vec4ClearColor.a);

        GLbitfield clearBits = 0;
        if (m_hasColorAttachment)
        {
            clearBits |= GL_COLOR_BUFFER_BIT;
        }
        if (m_hasDepthAttachment)
        {
            clearBits |= GL_DEPTH_BUFFER_BIT;
        }
        if (m_hasStencilAttachment)
        {
            clearBits |= GL_STENCIL_BUFFER_BIT;
        }
        glClear(clearBits);
    }

    std::shared_ptr<Texture> Framebuffer::GetTexture(int iIndex)
    {
        if (!m_vecAttachments.empty())
        {
            return m_vecAttachments[iIndex].Transform2Texture();
        }
        std::cout << "Framebuffer::GetTexture m_vecTextures is empty!" << std::endl;
        return nullptr;
    }

    std::vector<std::shared_ptr<Texture>> Framebuffer::GetAllTexture() const
    {
        if (m_vecAttachments.empty())
        {
            return std::vector<std::shared_ptr<Texture>>();
        }
        else
        {
            std::vector<std::shared_ptr<Texture>> vecResult;
            for (auto item : m_vecAttachments)
            {
                // ��Ⱦ���帽��ת����������
                if (item.m_eTarget != AttachmentTarget::TEXTURE)
                {
                    continue;
                }
                vecResult.push_back(item.Transform2Texture());
            }
            return vecResult;
        }
    }

}  // namespace Cme
