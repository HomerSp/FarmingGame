#version 330 core

out vec4 oColor;

uniform sampler2D iTexture;
varying vec2 vTexcoord;

void main(void)
{
    oColor = texture2D(iTexture, vTexcoord);
};