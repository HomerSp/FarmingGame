#version 330 core

uniform sampler2D uTexture;

out vec4 oColor;

in vec2 vTexCoords;

void main(void)
{
    oColor = texelFetch(uTexture, ivec2(vTexCoords), 0);
};