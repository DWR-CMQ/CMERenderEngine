#include <glad/glad.h>
#include "texture.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/gtc/type_ptr.hpp>
#include "common_helper.h"

namespace Cme 
{
    // C++中 静态函数中无法使用非静态函数
    void Texture::LoadTexture(const char* path, bool isSRGB)
    {
        TextureParams params;
        params.filtering = TextureFiltering::ANISOTROPIC;
        params.wrapMode = TextureWrapMode::REPEAT;

        m_eType = TextureType::TEXTURE_2D;

        stbi_set_flip_vertically_on_load(params.flipVerticallyOnLoad);
        unsigned char* data = stbi_load(path, &m_iWidth, &m_iHeight, &m_iNumChannels, 0);

        if (data == nullptr) 
        {
            stbi_image_free(data);
            throw TextureException("ERROR::TEXTURE::LOAD_FAILED\n" + std::string(path));
        }

        GLenum dataFormat;
        if (m_iNumChannels == 1)
        {
            m_uiInternalFormat = GL_R8;
            dataFormat = GL_RED;
        } 
        else if (m_iNumChannels == 2)
        {
            m_uiInternalFormat = GL_RG8;
            dataFormat = GL_RG;
        } 
        else if (m_iNumChannels == 3)
        {
            m_uiInternalFormat = isSRGB ? GL_SRGB8 : GL_RGB8;
            dataFormat = GL_RGB;
        }
        else if (m_iNumChannels == 4)
        {
            m_uiInternalFormat = isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            dataFormat = GL_RGBA;
        } 
        else 
        {
            stbi_image_free(data);
            throw TextureException(
                "ERROR::TEXTURE::UNSUPPORTED_TEXTURE_FORMAT\n"
                "Texture '" +
                std::string(path) + "' contained unsupported number of channels: " +
                std::to_string(m_iNumChannels));
        }

        glGenTextures(1, &m_uiID);
        glBindTexture(GL_TEXTURE_2D, m_uiID);

        // TODO: Replace with glTexStorage2D
        glTexImage2D(GL_TEXTURE_2D, 0, m_uiInternalFormat, m_iWidth, m_iHeight, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        if (params.generateMips >= MipGeneration::ON_LOAD)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
            m_iNumMips = CommonHelper::calculateNumMips(m_iWidth, m_iHeight);
            // TODO: Take into account params.maxNumMips
        } 
        else 
        {
            m_iNumMips = 1;
        }

        // Set texture-wrapping/filtering options.
        SetTextureParams(params, m_eType);

        stbi_image_free(data);
    }

    void Texture::LoadHDR(const char* path)
    {
        TextureParams params;

        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;

        m_eType = TextureType::TEXTURE_2D;
        m_iNumMips = 1;

        stbi_set_flip_vertically_on_load(true);
        float* data = stbi_loadf(path, &m_iWidth, &m_iHeight, &m_iNumChannels, 0);

        if (data == nullptr) 
        {
            stbi_image_free(data);
            throw TextureException("ERROR::TEXTURE::LOAD_FAILED\n" + std::string(path));
        }

        GLenum dataFormat;
        if (m_iNumChannels == 1)
        {
            m_uiInternalFormat = GL_R16F;
            dataFormat = GL_RED;
        } 
        else if (m_iNumChannels == 2)
        {
            m_uiInternalFormat = GL_RG16F;
            dataFormat = GL_RG;
        } 
        else if (m_iNumChannels == 3)
        {
            m_uiInternalFormat = GL_RGB16F;
            dataFormat = GL_RGB;
        } 
        else if (m_iNumChannels == 4)
        {
            m_uiInternalFormat = GL_RGBA16F;
            dataFormat = GL_RGBA;
        }
        else
        {
            stbi_image_free(data);
            throw TextureException(
                "ERROR::TEXTURE::UNSUPPORTED_TEXTURE_FORMAT\n"
                "Texture '" +
                std::string(path) + "' contained unsupported number of channels: " +
                std::to_string(m_iNumChannels));
        }

        glGenTextures(1, &m_uiID);
        glBindTexture(GL_TEXTURE_2D, m_uiID);

        // TODO: Replace with glTexStorage2D
        glTexImage2D(GL_TEXTURE_2D, 0, m_uiInternalFormat, m_iWidth, m_iHeight, 0, dataFormat, GL_FLOAT, data);

        // Set texture-wrapping/filtering options.
        SetTextureParams(params, TextureType::TEXTURE_2D);

        stbi_image_free(data);
    }

    void Texture::loadCubemap(std::vector<std::string> faces)
    {
        if (faces.size() != 6) 
        {
            throw TextureException(
                "ERROR::TEXTURE::INVALID_ARGUMENT\nMust pass exactly 6 faces to "
                "loadCubemap");
        }

        TextureParams params;
        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;

        glGenTextures(1, &m_uiID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiID);

        int width, height, numChannels;
        bool initialized = false;
        for (unsigned int i = 0; i < faces.size(); i++) 
        {
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &numChannels, 0);
            // Error handling.
            if (data == nullptr)
            {
                stbi_image_free(data);
                throw TextureException("ERROR::TEXTURE::LOAD_FAILED\n" + faces[i]);
            }
            if (numChannels != 3) 
            {
                stbi_image_free(data);
                throw TextureException(
                    "ERROR::TEXTURE::UNSUPPORTED_TEXTURE_FORMAT\n"
                    "Cubemap texture '" +
                    faces[i] + "' contained unsupported number of channels: " +
                    std::to_string(numChannels));
            }
            if (!initialized)
            {
                if (width != height) 
                {
                    throw TextureException(
                        "ERROR::TEXTURE::INVALID_TEXTURE_SIZE\n"
                        "Cubemap texture '" +
                        faces[i] + "' was not square");
                }
                m_iWidth = width;
                m_iHeight = height;
                m_iNumChannels = numChannels;
            } 
            else if (width != m_iWidth || height != m_iHeight)
            {
                throw TextureException(
                    "ERROR::TEXTURE::INVALID_TEXTURE_SIZE\n"
                    "Cubemap texture '" +
                    faces[i] + "' was a different size than the first face");
            }

            // Load into the next cube map texture position.
            // TODO: Replace with glTexStorage2D
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        SetTextureParams(params, TextureType::CUBEMAP);
    }

    void Texture::Create(int width, int height, GLenum internalFormat, const TextureParams& params, BufferType type)
    {
        GLenum textureTarget = GL_TEXTURE_2D;
        TextureType textureType = TextureType::TEXTURE_2D;
        if (type == BufferType::COLOR_CUBEMAP_HDR || type == BufferType::COLOR_CUBEMAP_HDR_ALPHA)
        {
            textureTarget = GL_TEXTURE_CUBE_MAP;
            textureType = TextureType::CUBEMAP;
        }

        m_eType = TextureType::TEXTURE_2D;
        m_iWidth = width;
        m_iHeight = height;
        m_iNumChannels = 0;  // Default.
        m_iNumMips = 1;
        if (params.generateMips == MipGeneration::ALWAYS) 
        {
            m_iNumMips = CommonHelper::calculateNumMips(m_iWidth, m_iHeight);
            if (params.maxNumMips >= 0) 
            {
                m_iNumMips = std::min(m_iNumMips, params.maxNumMips);
            }
        }
        m_uiInternalFormat = internalFormat;

        glGenTextures(1, &m_uiID);
        glBindTexture(textureTarget, m_uiID);

        glTexStorage2D(textureTarget, m_iNumMips, m_uiInternalFormat, m_iWidth, m_iHeight);

        // Set texture-wrapping/filtering options.
        SetTextureParams(params, textureType);
    }

    void Texture::createCubemap(int size, GLenum internalFormat)
    {
        TextureParams params;

        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;

        m_eType = TextureType::CUBEMAP;
        m_iWidth = size;
        m_iHeight = size;
        m_iNumChannels = 0;  // Default.
        m_iNumMips = 1;
        if (params.generateMips == MipGeneration::ALWAYS)
        {
            m_iNumMips = CommonHelper::calculateNumMips(m_iWidth, m_iHeight);
            if (params.maxNumMips >= 0) 
            {
                m_iNumMips = std::min(m_iNumMips, params.maxNumMips);
            }
        }
        m_uiInternalFormat = internalFormat;

        glGenTextures(1, &m_uiID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiID);

        glTexStorage2D(GL_TEXTURE_CUBE_MAP, m_iNumMips, m_uiInternalFormat, m_iWidth, m_iHeight);

        SetTextureParams(params, m_eType);
    }

    bool Texture::createFromData(int width, int height, GLenum internalFormat, const std::vector<glm::vec3>& data, const TextureParams& params)
    {
        if (data.size() != (width * height)) 
        {
            throw TextureException("ERROR::TEXTURE::INVALID_DATA_SIZE");
        }

        int iNumMips = 1;
        if (params.generateMips == MipGeneration::ALWAYS)
        {
            iNumMips = CommonHelper::calculateNumMips(width, height);
            if (params.maxNumMips >= 0)
            {
                iNumMips = std::min(iNumMips, params.maxNumMips);
            }
        }

        glGenTextures(1, &m_uiID);
        glBindTexture(GL_TEXTURE_2D, m_uiID);

        glTexStorage2D(GL_TEXTURE_2D, iNumMips, internalFormat, width, height);

        // Set texture-wrapping/filtering options.
        SetTextureParams(params, TextureType::TEXTURE_2D);

        // Upload the data. ssao专用
        //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_iWidth, m_iHeight, GL_RGB, GL_FLOAT, data.data());
        return true;
    }

    void Texture::BindToUnit(unsigned int textureUnit, TextureBindType bindType)
    {
        // TODO: Take into account GL_MAX_TEXTURE_UNITS here.
        glActiveTexture(GL_TEXTURE0 + textureUnit);

        if (bindType == TextureBindType::BY_TEXTURE_TYPE)
        {
            bindType = textureTypeToTextureBindType(m_eType);
        }

        switch (bindType) 
        {
        case TextureBindType::TEXTURE_2D:
            glBindTexture(GL_TEXTURE_2D, m_uiID);
            break;
        case TextureBindType::CUBEMAP:
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_uiID);
            break;
        case TextureBindType::IMAGE_TEXTURE:
            // Bind image unit.
            glBindImageTexture(textureUnit, m_uiID, 0, GL_FALSE, 0, GL_READ_WRITE, m_uiInternalFormat);
            break;
        default:
            throw TextureException("ERROR::TEXTURE::INVALID_TEXTURE_BIND_TYPE\n" +
                                    std::to_string(static_cast<int>(bindType)));
        }
    }

    void Texture::setSamplerMipRange(int min, int max)
    {
        GLenum target = textureTypeToGlTarget(m_eType);
        glBindTexture(target, m_uiID);
        glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, min);
        glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, max);
    }

    void Texture::unsetSamplerMipRange() 
    {
        // OpenGL defaults.
        setSamplerMipRange(0, 1000);
    }

    void Texture::free() 
    { 
        glDeleteTextures(1, &m_uiID);
    }

    void Texture::generateMips(int maxNumMips)
    {
        if (maxNumMips >= 0)
        {
            setSamplerMipRange(0, maxNumMips);
        }

        GLenum target = textureTypeToGlTarget(m_eType);
        glBindTexture(target, m_uiID);
        glGenerateMipmap(target);

        if (maxNumMips >= 0) 
        {
            unsetSamplerMipRange();
        }
    }

    void Texture::SetTextureParams(const TextureParams& params, TextureType type)
    {
        GLenum target = textureTypeToGlTarget(type);

        switch (params.filtering)
        {
        case TextureFiltering::NEAREST:
            glTexParameteri(target, 0x2801, GL_NEAREST);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case TextureFiltering::BILINEAR:
            glTexParameteri(target, 0x2801, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case TextureFiltering::TRILINEAR:
        case TextureFiltering::ANISOTROPIC:
            glTexParameteri(target, 0x2801, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            if (params.filtering == TextureFiltering::ANISOTROPIC)
            {
                constexpr float MAX_ANISOTROPY_SAMPLES = 4.0f;
                glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY,
                    MAX_ANISOTROPY_SAMPLES);
            }
            break;
        }

        switch (params.wrapMode)
        {
        case TextureWrapMode::REPEAT:
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
            if (type == TextureType::CUBEMAP)
            {
                glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_REPEAT);
            }
            break;
        case TextureWrapMode::CLAMP_TO_EDGE:
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            if (type == TextureType::CUBEMAP)
            {
                glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            }
            break;
        case TextureWrapMode::CLAMP_TO_BORDER:
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            if (type == TextureType::CUBEMAP)
            {
                glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
            }
            glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR,
                glm::value_ptr(params.borderColor));
            break;
        }
    }

} 
