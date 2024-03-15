#ifndef QUARKGL_SHADER_LOADER_H_
#define QUARKGL_SHADER_LOADER_H_

#include "exceptions.h"
#include "shader_defs.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Cme
{
    class ShaderLoaderException : public QuarkException 
    {
      using QuarkException::QuarkException;
    };

    class ShaderLoader 
    {
    public:
        ShaderLoader(const ShaderSource* shaderSource, const ShaderType type);
        std::string load();

    private:
        void checkShaderType(std::string const& shaderPath);
        bool alreadyLoadedOnce(std::string const& shaderPath);
        void cacheShaderCode(std::string const& shaderPath,
                            std::string const& shaderCode);
        std::string load(std::string const& shaderPath);
        std::string lookupOrLoad(std::string const& shaderPath);
        std::string preprocessShader(std::string const& shaderPath,
                                    std::string const& shaderCode);
        std::string getIncludesTraceback();
        bool checkCircularInclude(std::string const& resolvedPath);

        const ShaderSource* m_pShaderSource;
        const ShaderType m_eShaderType;
        std::deque<std::string> m_dequeIncludeChain;
        std::unordered_map<std::string, std::string> m_unmapCodeCache;
        std::unordered_set<std::string> m_unsetOnceCache;
    };
}  // namespace Cme

#endif
