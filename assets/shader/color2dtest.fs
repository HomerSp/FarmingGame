#version 330 core

out vec4 oColor;

in vec2 vCoords;
in vec4 vColor;

void main(void)
{
    oColor = vColor;
};