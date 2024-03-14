#ifndef QUARKGL_VERTEX_ARRAY_H_
#define QUARKGL_VERTEX_ARRAY_H_

#include <glad/glad.h>

#include <vector>

namespace qrk
{

class VertexArray
{
    public:
        VertexArray();
        // TODO: Can we have a destructor here?
        unsigned int getVao() { return m_uiVao; }
        unsigned int getVbo() { return m_uiVbo; }
        unsigned int getInstanceVbo() { return m_uiInstanceVbo; }
        unsigned int getEbo() { return m_uiEbo; }

        void activate();
        void deactivate();
        void loadVertexData(const std::vector<char>& data);
        void loadVertexData(const void* data, unsigned int size);
        void allocateInstanceVertexData(unsigned int size);
        void loadInstanceVertexData(const std::vector<char>& data);
        void loadInstanceVertexData(const void* data, unsigned int size);
        void loadElementData(const std::vector<unsigned int>& indices);
        void loadElementData(const unsigned int* indices, unsigned int size);
        void addVertexAttrib(unsigned int size, unsigned int type,
                            unsigned int instanceDivisor = 0);
        void finalizeVertexAttribs();

    private:
        struct VertexAttrib
        {
            unsigned int layoutPosition;
            unsigned int size;
            unsigned int type;
            unsigned int instanceDivisor;
        };

        unsigned int m_uiVao = 0;
        unsigned int m_uiVbo = 0;
        unsigned int m_uiInstanceVbo = 0;
        unsigned int m_uiEbo = 0;

        unsigned int m_uiVertexSizeBytes = 0;
        unsigned int m_uiElementSize = 0;

        std::vector<VertexAttrib> m_vecAttribs;
        unsigned int m_uiNextLayoutPosition = 0;
        unsigned int m_uiStride = 0;
    };
}  // namespace qrk

#endif
