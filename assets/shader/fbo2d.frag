#version 450

layout(set = 2, binding = 0) uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

layout(location = 0) in vec2 vTexCoords;

void main(void)
{
    oColor = texelFetch(uTexture, ivec2(vTexCoords), 0);
}
