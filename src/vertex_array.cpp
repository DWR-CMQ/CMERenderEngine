#include "vertex_array.h"

namespace Cme 
{
    VertexArray::VertexArray() 
    {
        glGenVertexArrays(1, &m_uiVao);
        activate();
    }

    void VertexArray::activate()
    { 
        glBindVertexArray(m_uiVao); 
    }

    void VertexArray::deactivate() 
    { 
        glBindVertexArray(0);
    }

    // vector类型数据
    void VertexArray::loadVertexData(const std::vector<char>& data)
    {
        activate();

        if (!m_uiVbo)
        {
            glGenBuffers(1, &m_uiVbo);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
        glBufferData(GL_ARRAY_BUFFER, data.size(), &data[0], GL_STATIC_DRAW);
        m_uiVertexSizeBytes = data.size();
    }

    // 指针类型数据
    void VertexArray::loadVertexData(const void* data, unsigned int sizeBytes) 
    {
        activate();

        if (!m_uiVbo)
        {
            glGenBuffers(1, &m_uiVbo);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_uiVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, GL_STATIC_DRAW);
        m_uiVertexSizeBytes = sizeBytes;
    }

    void VertexArray::allocateInstanceVertexData(unsigned int size) 
    {
        activate();

        if (!m_uiInstanceVbo)
        {
            glGenBuffers(1, &m_uiInstanceVbo);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_uiInstanceVbo);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
    }

    void VertexArray::loadInstanceVertexData(const std::vector<char>& data) 
    {
        activate();

        if (!m_uiInstanceVbo)
        {
            glGenBuffers(1, &m_uiInstanceVbo);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_uiInstanceVbo);
        glBufferData(GL_ARRAY_BUFFER, data.size(), &data[0], GL_STATIC_DRAW);
    }

    void VertexArray::loadInstanceVertexData(const void* data, unsigned int size)
    {
        activate();

        if (!m_uiInstanceVbo)
        {
            glGenBuffers(1, &m_uiInstanceVbo);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_uiInstanceVbo);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }

    void VertexArray::loadElementData(const std::vector<unsigned int>& indices) 
    {
        activate();

        if (!m_uiEbo)
        {
            glGenBuffers(1, &m_uiEbo);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size(), &indices[0], GL_STATIC_DRAW);
        m_uiElementSize = indices.size();
    }

    void VertexArray::loadElementData(const unsigned int* indices, unsigned int size) 
    {
        activate();

        if (!m_uiEbo)
        {
            glGenBuffers(1, &m_uiEbo);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
        m_uiElementSize = size;
    }

    void VertexArray::AddVertexAttrib(unsigned int size, unsigned int type, unsigned int instanceDivisor) 
    {
        VertexAttrib attrib;
        attrib.layoutPosition = m_uiNextLayoutPosition;
        attrib.size = size;
        attrib.type = type;
        attrib.instanceDivisor = instanceDivisor;


        m_vecAttribs.push_back(attrib);
        m_uiNextLayoutPosition++;
        // TODO: Support types other than float.
        m_uiStride += size * sizeof(float);
    }

    void VertexArray::SetVertexAttribs() 
    {
        activate();

        int offset = 0;
        for (const VertexAttrib& attrib : m_vecAttribs)
        {
            glVertexAttribPointer(attrib.layoutPosition, attrib.size, attrib.type, GL_FALSE, m_uiStride, static_cast<const char*>(nullptr) + offset);
            glEnableVertexAttribArray(attrib.layoutPosition);
            if (attrib.instanceDivisor)
            {
                glVertexAttribDivisor(attrib.layoutPosition, attrib.instanceDivisor);
            }
            offset += attrib.size * sizeof(float);
        }

        m_vecAttribs.clear();
        m_uiStride = 0;
    }
} 
