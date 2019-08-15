#version 330 core

uniform vec4 iColor;

out vec4 oColor;
in vec2 vCoords;

void main(void)
{
    oColor = iColor;
};