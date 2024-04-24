#include "../exceptions.h"
#include "shader_defs.h"
#include "shader_loader.h"
#include "../common_helper.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <iostream>

namespace Cme 
{

    std::string readFile(std::string const& path) 
    {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        file.open(path.c_str());
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void ShaderLoader::checkShaderType(std::string const& shaderPath)
    {
        // Allow ".glsl" as a generic shader suffix (e.g. for type-agnostic shader
        // code).
        if (CommonHelper::string_has_suffix(shaderPath, ".glsl")) return;

        switch (m_eShaderType)
        {
        case ShaderType::VERTEX:
            if (!CommonHelper::string_has_suffix(shaderPath, ".vert"))
            {
                throw ShaderLoaderException(
                    "ERROR::SHADER_LOADER::INVALID_EXTENSION\n"
                    "Loaded vertex shader '" +
                    shaderPath + "' must end in '.vert'.");
            }
            break;
        case ShaderType::FRAGMENT:
            if (!CommonHelper::string_has_suffix(shaderPath, ".frag"))
            {
                throw ShaderLoaderException(
                    "ERROR::SHADER_LOADER::INVALID_EXTENSION\n"
                    "Loaded fragment shader '" +
                    shaderPath + "' must end in '.frag'.");
            }
            break;
        case ShaderType::GEOMETRY:
            if (!CommonHelper::string_has_suffix(shaderPath, ".geom"))
            {
                throw ShaderLoaderException(
                    "ERROR::SHADER_LOADER::INVALID_EXTENSION\n"
                    "Loaded geometry shader '" +
                    shaderPath + "' must end in '.geom'.");
            }
            break;
        case ShaderType::COMPUTE:
            if (!CommonHelper::string_has_suffix(shaderPath, ".comp"))
            {
                throw ShaderLoaderException(
                    "ERROR::SHADER_LOADER::INVALID_EXTENSION\n"
                    "Loaded compute shader '" +
                    shaderPath + "' must end in '.comp'.");
            }
            break;
        }
    }

    bool ShaderLoader::alreadyLoadedOnce(std::string const& shaderPath) 
    {
        std::string resolvedPath = shaderPath;

        auto item = m_unsetOnceCache.find(resolvedPath);
        return item != m_unsetOnceCache.end();
    }

    std::string ShaderLoader::getIncludesTraceback() 
    {
        std::stringstream buffer;
        for (std::string path : m_dequeIncludeChain)
        {
            buffer << "  > " << path << std::endl;
        }
        return buffer.str();
    }

    bool ShaderLoader::checkCircularInclude(std::string const& resolvedPath) 
    {
        for (std::string path : m_dequeIncludeChain)
        {
            if (path == resolvedPath)
            {
                return true;
            }
        }
        return false;
    }

    ShaderLoader::ShaderLoader(const ShaderSource* shaderSource,
                               const ShaderType type)
        : m_pShaderSource(shaderSource), m_eShaderType(type) {}

    std::string ShaderLoader::lookupOrLoad(std::string const& shaderPath) 
    {
        //std::string resolvedPath = resolvePath(shaderPath);

        std::string resolvedPath = shaderPath;
        auto item = m_unmapCodeCache.find(resolvedPath);
        if (item != m_unmapCodeCache.end())
        {
            // Cache hit; return code.
            return item->second;
        }

        // Cache miss; read code from file.
        std::string shaderCode;
        try 
        {
            shaderCode = readFile(shaderPath);
        } 
        catch (std::ifstream::failure& e)
        {
            std::string traceback = getIncludesTraceback();
            throw ShaderLoaderException(
                "ERROR::SHADER_LOADER::FILE_NOT_SUCCESSFULLY_READ\n"
                "Unable to read shader '" +
                std::string(shaderPath) + "', traceback below (most recent last):\n" +
                traceback);
        }
        // Don't cache the code just yet, because we need to preprocess it.
        return shaderCode;
    }

    std::string ShaderLoader::load(std::string const& shaderPath)
    {
        checkShaderType(shaderPath);

        // Handle #pragma once.
        if (alreadyLoadedOnce(shaderPath))
        {
            return "";
        }

        auto shaderCode = lookupOrLoad(shaderPath);
        auto processedCode = preprocessShader(shaderPath, shaderCode);
        cacheShaderCode(shaderPath, processedCode);
        return processedCode;
    }

    void ShaderLoader::cacheShaderCode(std::string const& shaderPath,
                                       std::string const& shaderCode)
    {
        //std::string resolvedPath = resolvePath(shaderPath);

        std::string resolvedPath = shaderPath;
        m_unmapCodeCache[resolvedPath] = shaderCode;
    }

    std::string ShaderLoader::preprocessShader(std::string const& shaderPath,
                                               std::string const& shaderCode)
    {
        std::string resolvedPath = shaderPath;
        m_dequeIncludeChain.push_back(resolvedPath);

        std::regex oncePattern(R"(((^|\r?\n)\s*)#pragma\s+once\s*(?=\r?\n|$))");
        if (std::regex_search(shaderCode, oncePattern)) 
        {
            m_unsetOnceCache.insert(resolvedPath);
        }

        std::regex includePattern(R"(((^|\r?\n)\s*)#pragma\s+qrk_include\s+(".*"|<.*>)(?=\r?\n|$))");
        std::string processedCode = CommonHelper::regexReplace(shaderCode, includePattern, [this, shaderPath](const std::smatch& m)
        {
            std::string whitespace = m[1];
            // Extract the path.
            std::string incl = m[3];
            char inclType = incl[0];
            std::string path = CommonHelper::trim(incl.substr(1, incl.size() - 2));

            if (inclType == '<')
            {
                // Cme include.
                return whitespace + load("assets/shaders/" + path);
            } 
            else
            {
                // Standard include.
                size_t i = shaderPath.find_last_of("/");
                // This will either be the current shader's directory, or empty
                // string if the current shader is at project root.
                std::string prefix =
                    i != std::string::npos ? shaderPath.substr(0, i + 1) : "";
                return whitespace + load(prefix + path);
            }
        });

        //std::string processedCode = shaderCode;
        m_dequeIncludeChain.pop_back();
        return processedCode;
    }

    std::string ShaderLoader::load()
    {
        // Handle either loading from file, or loading from inline source.
        m_unsetOnceCache.clear();
        if (m_pShaderSource->isPath())
        {
            return load(m_pShaderSource->value);
        } 
        else
        {
            return preprocessShader(".", m_pShaderSource->value);
        }
    }
}  // namespace Cme
