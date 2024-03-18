#include <glad/glad.h>
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/gtc/type_ptr.hpp>

namespace Cme 
{
    int calculateNumMips(int width, int height) 
    {
        return 1 + static_cast<int>(std::floor(std::log2(std::max(width, height))));
    }

    ImageSize calculateNextMip(const ImageSize& mipSize)
    {
        ImageSize temp;
        temp.width = std::max(mipSize.width / 2, 1);
        temp.height = std::max(mipSize.height / 2, 1);
        return temp;
    }

    ImageSize calculateMipLevel(int mip0Width, int mip0Height, int level)
    {
        ImageSize size = {mip0Width, mip0Height};
        if (level == 0)
        {
            return size;
        }
        for (int mip = 0; mip < level; ++mip) 
        {
            size = calculateNextMip(size);
        }
        return size;
    }

    Texture Texture::load(const char* path, bool isSRGB)
    {
        TextureParams params;
        params.filtering = TextureFiltering::ANISOTROPIC;
        params.wrapMode = TextureWrapMode::REPEAT;
        return load(path, isSRGB, params);
    }

    Texture Texture::load(const char* path, bool isSRGB,
                          const TextureParams& params)
    {
        Texture texture;
        texture.m_eType = TextureType::TEXTURE_2D;

        stbi_set_flip_vertically_on_load(params.flipVerticallyOnLoad);
        unsigned char* data =
            stbi_load(path, &texture.m_iWidth, &texture.m_iHeight, &texture.m_iNumChannels,
                    /*desired_channels=*/0);

        if (data == nullptr) 
        {
            stbi_image_free(data);
            throw TextureException("ERROR::TEXTURE::LOAD_FAILED\n" + std::string(path));
        }

        GLenum dataFormat;
        if (texture.m_iNumChannels == 1)
        {
            texture.m_uiInternalFormat = GL_R8;
            dataFormat = GL_RED;
        } 
        else if (texture.m_iNumChannels == 2)
        {
            texture.m_uiInternalFormat = GL_RG8;
            dataFormat = GL_RG;
        } 
        else if (texture.m_iNumChannels == 3)
        {
            texture.m_uiInternalFormat = isSRGB ? GL_SRGB8 : GL_RGB8;
            dataFormat = GL_RGB;
        }
        else if (texture.m_iNumChannels == 4)
        {
            texture.m_uiInternalFormat = isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            dataFormat = GL_RGBA;
        } 
        else 
        {
            stbi_image_free(data);
            throw TextureException(
                "ERROR::TEXTURE::UNSUPPORTED_TEXTURE_FORMAT\n"
                "Texture '" +
                std::string(path) + "' contained unsupported number of channels: " +
                std::to_string(texture.m_iNumChannels));
        }

        glGenTextures(1, &texture.m_uiID);
        glBindTexture(GL_TEXTURE_2D, texture.m_uiID);

        // TODO: Replace with glTexStorage2D
        glTexImage2D(GL_TEXTURE_2D, /* mipmap level */ 0, texture.m_uiInternalFormat,
                    texture.m_iWidth, texture.m_iHeight, 0,
                    /* tex data format */ dataFormat, GL_UNSIGNED_BYTE, data);
        if (params.generateMips >= MipGeneration::ON_LOAD)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
            texture.m_iNumMips = calculateNumMips(texture.m_iWidth, texture.m_iHeight);
            // TODO: Take into account params.maxNumMips
        } 
        else 
        {
            texture.m_iNumMips = 1;
        }

        // Set texture-wrapping/filtering options.
        applyParams(params, texture.m_eType);

        stbi_image_free(data);

        return texture;
    }

    Texture Texture::LoadHDR(const char* path)
    {
        TextureParams params;

        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
        return LoadHDR(path, params);
    }

    Texture Texture::LoadHDR(const char* path, const TextureParams& params)
    {
        Texture texture;
        texture.m_eType = TextureType::TEXTURE_2D;
        texture.m_iNumMips = 1;

        stbi_set_flip_vertically_on_load(true);
        float* data = stbi_loadf(path, &texture.m_iWidth, &texture.m_iHeight,
                                &texture.m_iNumChannels, /*desired_channels=*/0);

        if (data == nullptr) 
        {
            stbi_image_free(data);
            throw TextureException("ERROR::TEXTURE::LOAD_FAILED\n" + std::string(path));
        }

        GLenum dataFormat;
        if (texture.m_iNumChannels == 1)
        {
            texture.m_uiInternalFormat = GL_R16F;
            dataFormat = GL_RED;
        } 
        else if (texture.m_iNumChannels == 2)
        {
            texture.m_uiInternalFormat = GL_RG16F;
            dataFormat = GL_RG;
        } 
        else if (texture.m_iNumChannels == 3)
        {
            texture.m_uiInternalFormat = GL_RGB16F;
            dataFormat = GL_RGB;
        } 
        else if (texture.m_iNumChannels == 4)
        {
            texture.m_uiInternalFormat = GL_RGBA16F;
            dataFormat = GL_RGBA;
        }
        else
        {
            stbi_image_free(data);
            throw TextureException(
                "ERROR::TEXTURE::UNSUPPORTED_TEXTURE_FORMAT\n"
                "Texture '" +
                std::string(path) + "' contained unsupported number of channels: " +
                std::to_string(texture.m_iNumChannels));
        }

        glGenTextures(1, &texture.m_uiID);
        glBindTexture(GL_TEXTURE_2D, texture.m_uiID);

        // TODO: Replace with glTexStorage2D
        glTexImage2D(GL_TEXTURE_2D, /*mip=*/0, texture.m_uiInternalFormat,
                    texture.m_iWidth, texture.m_iHeight, 0,
                    /*tex data format=*/dataFormat, GL_FLOAT, data);

        // Set texture-wrapping/filtering options.
        applyParams(params, texture.m_eType);

        stbi_image_free(data);

        return texture;
    }

    Texture Texture::loadCubemap(std::vector<std::string> faces) 
    {
        TextureParams params;
        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;

        return loadCubemap(faces, params);
    }

    Texture Texture::loadCubemap(std::vector<std::string> faces,
                                 const TextureParams& params)
    {
        if (faces.size() != 6) 
        {
        throw TextureException(
            "ERROR::TEXTURE::INVALID_ARGUMENT\nMust pass exactly 6 faces to "
            "loadCubemap");
        }

        Texture texture;
        texture.m_eType = TextureType::CUBEMAP;
        texture.m_iNumMips = 1;
        texture.m_uiInternalFormat = GL_RGB8;  // Cubemaps must be RGB.

        glGenTextures(1, &texture.m_uiID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture.m_uiID);

        int width, height, numChannels;
        bool initialized = false;
        for (unsigned int i = 0; i < faces.size(); i++) 
        {
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height,
                                            &numChannels, /*desired_channels=*/0);
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
                texture.m_iWidth = width;
                texture.m_iHeight = height;
                texture.m_iNumChannels = numChannels;
            } 
            else if (width != texture.m_iWidth || height != texture.m_iHeight)
            {
                throw TextureException(
                    "ERROR::TEXTURE::INVALID_TEXTURE_SIZE\n"
                    "Cubemap texture '" +
                    faces[i] + "' was a different size than the first face");
            }

            // Load into the next cube map texture position.
            // TODO: Replace with glTexStorage2D
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, /*level=*/0,
                            texture.m_uiInternalFormat, width, height, /*border=*/0,
                            /*format=*/GL_RGB, /*type=*/GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        applyParams(params, texture.m_eType);

        return texture;
    }

    Texture Texture::create(int width, int height, GLenum internalFormat) 
    {
        TextureParams params;

        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
        return create(width, height, internalFormat, params);
    }

    Texture Texture::create(int width, int height, GLenum internalFormat,
                            const TextureParams& params) 
    {
        Texture texture;
        texture.m_eType = TextureType::TEXTURE_2D;
        texture.m_iWidth = width;
        texture.m_iHeight = height;
        texture.m_iNumChannels = 0;  // Default.
        texture.m_iNumMips = 1;
        if (params.generateMips == MipGeneration::ALWAYS) 
        {
            texture.m_iNumMips = calculateNumMips(texture.m_iWidth, texture.m_iHeight);
            if (params.maxNumMips >= 0) 
            {
                texture.m_iNumMips = std::min(texture.m_iNumMips, params.maxNumMips);
            }
        }
        texture.m_uiInternalFormat = internalFormat;

        glGenTextures(1, &texture.m_uiID);
        glBindTexture(GL_TEXTURE_2D, texture.m_uiID);

        glTexStorage2D(GL_TEXTURE_2D, texture.m_iNumMips, texture.m_uiInternalFormat,
                        texture.m_iWidth, texture.m_iHeight);

        // Set texture-wrapping/filtering options.
        applyParams(params, texture.m_eType);

        return texture;
    }

    Texture Texture::createCubemap(int size, GLenum internalFormat)
    {
        TextureParams params;

        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
        return createCubemap(size, internalFormat, params);
    }

    Texture Texture::createCubemap(int size, GLenum internalFormat,
                                   const TextureParams& params)
    {
        Texture texture;
        texture.m_eType = TextureType::CUBEMAP;
        texture.m_iWidth = size;
        texture.m_iHeight = size;
        texture.m_iNumChannels = 0;  // Default.
        texture.m_iNumMips = 1;
        if (params.generateMips == MipGeneration::ALWAYS)
        {
            texture.m_iNumMips = calculateNumMips(texture.m_iWidth, texture.m_iHeight);
            if (params.maxNumMips >= 0) 
            {
                texture.m_iNumMips = std::min(texture.m_iNumMips, params.maxNumMips);
            }
        }
        texture.m_uiInternalFormat = internalFormat;

        glGenTextures(1, &texture.m_uiID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture.m_uiID);

        glTexStorage2D(GL_TEXTURE_CUBE_MAP, texture.m_iNumMips, texture.m_uiInternalFormat,
                        texture.m_iWidth, texture.m_iHeight);

        applyParams(params, texture.m_eType);

        return texture;
    }

    Texture Texture::createFromData(int width, int height, GLenum internalFormat,
                                    const std::vector<glm::vec3>& data) 
    {
        TextureParams params;

        params.filtering = TextureFiltering::BILINEAR;
        params.wrapMode = TextureWrapMode::CLAMP_TO_EDGE;

        return createFromData(width, height, internalFormat, data, params);
    }

    Texture Texture::createFromData(int width, int height, GLenum internalFormat,
                                    const std::vector<glm::vec3>& data,
                                    const TextureParams& params)
    {
        if (data.size() != (width * height)) 
        {
            throw TextureException("ERROR::TEXTURE::INVALID_DATA_SIZE");
        }

        Texture texture = Texture::create(width, height, internalFormat, params);
        // Upload the data.
        glTexSubImage2D(GL_TEXTURE_2D, /*level=*/0, /*xoffset=*/0,
                        /*yoffset=*/0, texture.m_iWidth, texture.m_iHeight,
                        /*format=*/GL_RGB, GL_FLOAT, data.data());
        return texture;
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

    void Texture::applyParams(const TextureParams& params, TextureType type) 
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
