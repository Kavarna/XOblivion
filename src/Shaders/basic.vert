#version 450


layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;


layout(binding = 0) uniform UniformBufferObject
{
    mat4 world;
    mat4 view;
    mat4 projection;
} ubo;

void main()
{
    vec4 finalPosition = inPosition;
    gl_Position = ubo.projection * ubo.view * ubo.world * finalPosition;
    outColor = inColor;
}