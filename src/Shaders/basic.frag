#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 inColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform textureInfo
{
    uint hasTexture;
} u_textureInfo;

void main()
{
    if (u_textureInfo.hasTexture == 0)
    {
        outColor = inColor;
    }
    else
    {
        outColor = texture(texSampler, inColor.xy);
    }
}