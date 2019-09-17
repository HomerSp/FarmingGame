#version 330 core

uniform sampler2DArray uTexture;

out vec4 oColor;

in vec2 vTexCoords;
in float vTexLayer;

void main(void)
{
    vec4 c = texelFetch(uTexture, ivec3(vTexCoords, vTexLayer), 0);
    if (c.a == 0.0) {
        discard;
    }

    oColor = c;
};