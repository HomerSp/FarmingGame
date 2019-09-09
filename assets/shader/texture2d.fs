#version 330 core

uniform sampler2D iTexture;

out vec4 oColor;
in vec2 vCoords;

void main(void)
{
    ivec2 r = textureSize(iTexture, 0);
    oColor = texture2D(iTexture, vCoords / r);
};