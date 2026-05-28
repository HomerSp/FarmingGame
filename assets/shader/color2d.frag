#version 450

layout(location = 0) out vec4 oColor;

layout(location = 0) in vec2 vCoords;
layout(location = 1) in vec4 vColor;

void main(void)
{
    oColor = vColor;
}
