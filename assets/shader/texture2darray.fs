#version 330 core

uniform sampler2DArray iTexture;

out vec4 oColor;

in vec2 vCoords;
in float vLayer;

void main(void)
{
    oColor = texelFetch(iTexture, ivec3(vCoords, vLayer), 0);
};