#include "Model.h"

#include "Utils/ObjLoader.h"
#include "Utils/VulkanAllocators.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


namespace std
{
    template <>
    struct hash<PositionColorVertex>
    {
        size_t operator()(const PositionColorVertex& t) const
        {
            return (hash<glm::vec4>()(t.position) << 2) ^
                (hash<glm::vec4>()(t.color) >> 1);
        }
    };
}

Model::Model(const char* path)
{
    createFromPath(path);
}

Model::~Model()
{
    if (m_vertexBuffer.m_buffer)
    {
        m_vulkanDevice.m_logicalDevice.destroyBuffer(m_vertexBuffer.m_buffer);
        vmaFreeMemory(g_allocator, m_vertexBuffer.m_memory);
        m_vertexBuffer.m_buffer = nullptr;
    }
    if (m_indexBuffer.m_buffer)
    {
        m_vulkanDevice.m_logicalDevice.destroyBuffer(m_indexBuffer.m_buffer);
        vmaFreeMemory(g_allocator, m_indexBuffer.m_memory);
        m_indexBuffer.m_buffer = nullptr;
    }
}

auto Model::bind(vk::CommandBuffer commands) -> void
{
    std::array<vk::Buffer, 1> vertexBuffers = { m_vertexBuffer.m_buffer };
    std::array<vk::DeviceSize, 1> vertexBuffersOffsets = { 0 };
    commands.bindVertexBuffers(0, vertexBuffers, vertexBuffersOffsets);
    commands.bindIndexBuffer(m_indexBuffer.m_buffer, 0, vk::IndexType::eUint32);
}

auto Model::createFromPath(const char* path) -> void
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::material_t> materials;
    std::vector<tinyobj::shape_t> shapes;
    std::string warn, err;
    EVALUATE(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path), false,
        == , "Couldn't load model from path %s", path);

    std::unordered_map<PositionColorVertex, uint32_t> uniqueVertices = {};
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            PositionColorVertex vertex;
            vertex.position =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
                1.0f
            };

            if (index.texcoord_index >= 0)
            {
                vertex.color =
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.f - attrib.texcoords[2 * index.texcoord_index + 1],
                    0.0f,
                    1.0f
                };
            }
            else
            {
                vertex.color =
                {
                    attrib.colors[2 * index.vertex_index + 0],
                    attrib.colors[2 * index.vertex_index + 1],
                    attrib.colors[2 * index.vertex_index + 2],
                    1.0f
                };
            }

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = (uint32_t)m_vertices.size();
                m_vertices.push_back(vertex);
            }

            m_indices.push_back(uniqueVertices[vertex]);
        }
    }

    uint32_t numVertices = (uint32_t)m_vertices.size();

    m_vertexBuffer = BufferUtils::createBuffer(vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, {},
        &m_vulkanDevice.m_families.graphicsIndex, 1,
        numVertices * PositionColorVertex::getVertexSize(),
        m_vulkanDevice.m_logicalDevice, m_vulkanDevice.m_queues.graphicsQueue,
        m_vulkanDevice.m_families.graphicsIndex,
        (void*)m_vertices.data());

    uint32_t numIndices = (uint32_t)m_indices.size();
    m_indexBuffer = BufferUtils::createBuffer(vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, {},
        &m_vulkanDevice.m_families.graphicsIndex, 1,
        numIndices * sizeof(m_indices[0]),
        m_vulkanDevice.m_logicalDevice, m_vulkanDevice.m_queues.graphicsQueue,
        m_vulkanDevice.m_families.graphicsIndex, (void*)m_indices.data());
}
