// STL
#include <iostream>
#include <cstring>

// Project
#include "vertex_buffer_object.h"

void VertexBufferObject::CreateVBO(size_t reserveSizeBytes)
{
    if (isBufferCreated())
    {
        std::cerr << "This buffer is already created! You need to delete it before re-creating it!" << std::endl;
        return;
    }

    glGenBuffers(1, &m_iBufferID);
    m_vecRawData.reserve(reserveSizeBytes > 0 ? reserveSizeBytes : 1024);
    std::cout << "Created vertex buffer object with ID " << m_iBufferID << " and initial reserved size " << m_vecRawData.capacity() << " bytes" << std::endl;
}

void VertexBufferObject::BindVBO(GLenum bufferType)
{
    if (!isBufferCreated())
    {
        std::cerr << "This buffer is not created yet! You cannot bind it before you create it!" << std::endl;
        return;
    }

    m_uiBufferType = bufferType;
    glBindBuffer(m_uiBufferType, m_iBufferID);
}

void VertexBufferObject::AddRawData(const void* ptrData, size_t dataSizeBytes, size_t repeat)
{
    const auto bytesToAdd = dataSizeBytes * repeat;
    const auto requiredCapacity = m_uiBytesAdded + bytesToAdd;

    // If the current capacity isn't sufficient, let's resize the internal vector of raw data
    if (requiredCapacity > m_vecRawData.capacity())
    {
        // Determine new raw data buffer capacity - enlarge by a factor of two until it becomes big enough
        auto newCapacity = m_vecRawData.capacity() * 2;
        while (newCapacity < requiredCapacity)
        {
            newCapacity *= 2;
        }

        // Reserve new vector with higher capacity (which will replace member rawData_) and copy existing data over
        std::vector<unsigned char> newRawData;
        newRawData.reserve(newCapacity);
        memcpy(newRawData.data(), m_vecRawData.data(), m_uiBytesAdded);
        m_vecRawData = std::move(newRawData);
    }

    // Add the data now that we are sure that capacity is sufficient
    for (size_t i = 0; i < repeat; i++)
    {
        memcpy(m_vecRawData.data() + m_uiBytesAdded, ptrData, dataSizeBytes);
        m_uiBytesAdded += dataSizeBytes;
    }
}

void* VertexBufferObject::GetRawDataPointer()
{
    return m_vecRawData.data();
}

void VertexBufferObject::UploadDataToGPU(GLenum usageHint)
{
    if (!isBufferCreated())
    {
        std::cerr << "This buffer is not created yet! Call createVBO before uploading data to GPU!" << std::endl;
        return;
    }

    glBufferData(m_uiBufferType, m_uiBytesAdded, m_vecRawData.data(), usageHint);
    m_uiUploadedDataSize = m_uiBytesAdded;
    m_uiBytesAdded = 0;
}

void* VertexBufferObject::MapBufferToMemory(GLenum usageHint) const
{
    if (!isDataUploaded()) 
    {
        return nullptr;
    }

    return glMapBuffer(m_uiBufferType, usageHint);
}

void* VertexBufferObject::MapSubBufferToMemory(GLenum usageHint, size_t offset, size_t length) const
{
    if (!isDataUploaded())
    {
        return nullptr;
    }

    return glMapBufferRange(m_uiBufferType, offset, length, usageHint);
}

void VertexBufferObject::UnmapBuffer() const
{
    glUnmapBuffer(m_uiBufferType);
}

GLuint VertexBufferObject::GetBufferID() const
{
    return m_iBufferID;
}

size_t VertexBufferObject::GetBufferSize()
{
    return isDataUploaded() ? m_uiUploadedDataSize : m_uiBytesAdded;
}

void VertexBufferObject::DeleteVBO()
{
    if (!isBufferCreated()) 
    {
        return;
    }

    std::cout << "Deleting vertex buffer object with ID " << m_iBufferID << "..." << std::endl;
    glDeleteBuffers(1, &m_iBufferID);
    m_iBufferID = 0;
    m_uiBytesAdded = 0;
    m_uiUploadedDataSize = 0;
}

bool VertexBufferObject::isBufferCreated() const
{
    return m_iBufferID != 0;
}

bool VertexBufferObject::isDataUploaded() const
{
    return m_uiUploadedDataSize > 0;
}
