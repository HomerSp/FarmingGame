#version 330 core

out vec4 oColor;

uniform vec4 iColor;

varying vec2 vTexcoord;

void main(void)
{
    oColor = iColor;
};