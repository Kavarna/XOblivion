#pragma once


#include "Interfaces/IGraphicsObject.h"
#include "Utils/BufferUtils.h"

#include "Vertex/PositionColorVertex.h"


class Model : public IVulkanDeviceObject
{
public:
    Model(const char* path);
    ~Model();

public:
    auto                                bind(vk::CommandBuffer) -> void;
    auto                                getVertexCount() -> uint32_t { return (uint32_t)m_vertices.size(); };
    auto                                getIndexCount() -> uint32_t { return (uint32_t)m_indices.size(); };

private:
    auto                                createFromPath(const char* path) -> void;


private:
    BufferUtils::Buffer                 m_vertexBuffer;
    BufferUtils::Buffer                 m_indexBuffer;

    std::vector<PositionColorVertex>    m_vertices;
    std::vector<uint32_t>               m_indices;
};