#ifndef QUARKGL_SHADER_H_
#define QUARKGL_SHADER_H_

#include <glad/glad.h>
#include "../exceptions.h"
#include "shader_defs.h"
#include "../core/texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Cme 
{
    class Shader;

    class ShaderException : public QuarkException
    {
        using QuarkException::QuarkException;
    };

    class Shader 
    {
    public:
        Shader(const ShaderSource& vertexSource, const ShaderSource& fragmentSource);
        Shader(const ShaderSource& vertexSource, const ShaderSource& fragmentSource, const ShaderSource& geometrySource);
        Shader(const ShaderSource& computerSource);
        virtual ~Shader() = default;

        unsigned int getProgramId() const { return m_uiShaderProgramID; }

        virtual void activate();
        virtual void deactivate();

        // Functions for uniforms.

        virtual void setBool(const char* name, bool value);
        void setBool(std::string name, bool value) { setBool(name.c_str(), value); }

        virtual void setUInt(const char* name, unsigned int value);
        void setUInt(std::string name, unsigned int value)
        {
            setUInt(name.c_str(), value);
        }

        virtual void setInt(const char* name, int value);
        void setInt(std::string name, int value) { setInt(name.c_str(), value); }

        virtual void setFloat(const char* name, float value);
        void setFloat(std::string name, float value) 
        {
            setFloat(name.c_str(), value);
        }

        virtual void setVec3(const char* name, const glm::vec3& vector);
        void setVec3(std::string name, const glm::vec3& vector) 
        {
            setVec3(name.c_str(), vector);
        }

        virtual void setVec3(const char* name, float v0, float v1, float v2);
        void setVec3(std::string name, float v0, float v1, float v2) 
        {
            setVec3(name.c_str(), v0, v1, v2);
        }

        void setVec4(const char* name, const glm::vec4& vector);

        void setVec4(const char* name, float v0, float v1, float v2, float w);

        virtual void setMat4(const char* name, const glm::mat4& matrix);
        void setMat4(std::string name, const glm::mat4& matrix) 
        {
            setMat4(name.c_str(), matrix);
        }

        void bind(GLuint id, int val) const;
        void bind(std::string const& name, int val) const;
        void bind(const char* name, int val) const;

        void output(std::string const& out);

    protected:
        Shader() = default;
        int safeGetUniformLocation(const char* name);

        unsigned int m_uiShaderProgramID;
    private:
        void CheckCompileErrors(unsigned int shader, std::string type);
    };

}  // namespace Cme

#endif
