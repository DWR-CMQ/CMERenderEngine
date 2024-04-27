#include "shader.h"
#include "shader_loader.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace Cme
{
    Shader::Shader(const ShaderSource& vertexSource, const ShaderSource& fragmentSource) 
    {
        ShaderLoader vshaderLoader(&vertexSource, ShaderType::VERTEX);
        ShaderLoader fshaderLoader(&fragmentSource, ShaderType::FRAGMENT);
        std::string vertexCode = vshaderLoader.load();
        std::string fragmentCode = fshaderLoader.load();
        const char* vCode = vertexCode.c_str();
        const char* fCode = fragmentCode.c_str();

        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vCode, nullptr);
        glCompileShader(vertex);
        CheckCompileErrors(vertex, "VERTEX");

        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fCode, nullptr);
        glCompileShader(fragment);
        CheckCompileErrors(fragment, "FRAGMENT");

        m_uiShaderProgramID = glCreateProgram();
        glAttachShader(m_uiShaderProgramID, vertex);
        glAttachShader(m_uiShaderProgramID, fragment);
        glLinkProgram(m_uiShaderProgramID);
        CheckCompileErrors(m_uiShaderProgramID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    Shader::Shader(const ShaderSource& vertexSource,
                   const ShaderSource& fragmentSource,
                   const ShaderSource& geometrySource)
    {
        ShaderLoader vshaderLoader(&vertexSource, ShaderType::VERTEX);
        ShaderLoader fshaderLoader(&fragmentSource, ShaderType::FRAGMENT);
        ShaderLoader gshaderLoader(&geometrySource, ShaderType::GEOMETRY);
        std::string vertexCode = vshaderLoader.load();
        std::string fragmentCode = fshaderLoader.load();
        std::string geometryCode = gshaderLoader.load();
        const char* vCode = vertexCode.c_str();
        const char* fCode = fragmentCode.c_str();
        const char* gCode = geometryCode.c_str();

        unsigned int vertex, fragment, geometry;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vCode, nullptr);
        glCompileShader(vertex);
        CheckCompileErrors(vertex, "VERTEX");

        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fCode, nullptr);
        glCompileShader(fragment);
        CheckCompileErrors(fragment, "FRAGMENT");

        // geometry Shader
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gCode, nullptr);
        glCompileShader(geometry);
        CheckCompileErrors(geometry, "GEOMETRY");

        m_uiShaderProgramID = glCreateProgram();
        glAttachShader(m_uiShaderProgramID, vertex);
        glAttachShader(m_uiShaderProgramID, fragment);
        glAttachShader(m_uiShaderProgramID, geometry);

        glLinkProgram(m_uiShaderProgramID);
        CheckCompileErrors(m_uiShaderProgramID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteShader(geometry);
    }

    /// ¼ÆËã×ÅÉ«Æ÷
    Shader::Shader(const ShaderSource& computerSource)
    {
        ShaderLoader cshaderLoader(&computerSource, ShaderType::COMPUTE);
        std::string computetCode = cshaderLoader.load();
        const char* cCode = computetCode.c_str();

        unsigned int compute;

        // geometry Shader
        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cCode, nullptr);
        glCompileShader(compute);
        CheckCompileErrors(compute, "COMPUTE");

        m_uiShaderProgramID = glCreateProgram();
        glAttachShader(m_uiShaderProgramID, compute);

        glLinkProgram(m_uiShaderProgramID);
        CheckCompileErrors(m_uiShaderProgramID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(compute);
    }

    int Shader::safeGetUniformLocation(const char* name) 
    {
        int uniform = glGetUniformLocation(m_uiShaderProgramID, name);
        if (uniform == -1) 
        {
        // TODO: Log a message; either uniform is invalid, or it got optimized away
        // by the shader.
        // printf("unknown uniform: %s\n", name);
        }
        return uniform;
    }

    void Shader::activate() 
    { 
        glUseProgram(m_uiShaderProgramID);
    }
    void Shader::deactivate() { glUseProgram(0); }


    void Shader::setBool(const char* name, bool value) 
    {
        activate();
        glUniform1i(safeGetUniformLocation(name), static_cast<int>(value));
    }

    void Shader::setUInt(const char* name, unsigned int value)
    {
        activate();
        glUniform1ui(safeGetUniformLocation(name), value);
    }

    void Shader::setInt(const char* name, int value)
    {
        activate();
        glUniform1i(safeGetUniformLocation(name), value);
    }

    void Shader::setFloat(const char* name, float value) 
    {
        activate();
        glUniform1f(safeGetUniformLocation(name), value);
    }

    void Shader::setVec3(const char* name, const glm::vec3& vector)
    {
        activate();
        glUniform3fv(safeGetUniformLocation(name), /*count=*/1,
                    glm::value_ptr(vector));
    }

    void Shader::setVec3(const char* name, float v0, float v1, float v2) 
    {
        activate();
        glUniform3f(safeGetUniformLocation(name), v0, v1, v2);
    }

    void Shader::setMat4(const char* name, const glm::mat4& matrix) 
    {
        activate();
        glUniformMatrix4fv(safeGetUniformLocation(name), /*count=*/1,
                            /*transpose=*/GL_FALSE, glm::value_ptr(matrix));
    }

    void Shader::bind(GLuint id, int val) const
    {
        glUniformBlockBinding(m_uiShaderProgramID, id, val);
    }

    void Shader::bind(std::string const& name, int val) const
    {
        glUniformBlockBinding(m_uiShaderProgramID, glGetUniformBlockIndex(m_uiShaderProgramID, name.c_str()), val);
    }

    void Shader::bind(const char* name, int val) const
    {
        glUniformBlockBinding(m_uiShaderProgramID, glGetUniformBlockIndex(m_uiShaderProgramID, name), val);
    }

    void Shader::output(std::string const& out)
    {
        glBindFragDataLocation(m_uiShaderProgramID, 0, "color");
    }

    void Shader::CheckCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }

}  // namespace Cme
