#version 450

layout(set = 2, binding = 0) uniform sampler2DArray uTexture;

layout(location = 0) out vec4 oColor;

layout(location = 0) in vec2 vTexCoords;
layout(location = 1) in float vTexLayer;

void main(void)
{
    vec4 c = texelFetch(uTexture, ivec3(vTexCoords, vTexLayer), 0);
    if (c.a == 0.0) {
        discard;
    }

    oColor = c;
}
