#version 330 core

layout(location = 0) in vec2 iCoords;
layout(location = 1) in vec3 iVertex;
uniform mat4 iMatrix;

varying vec2 vTexcoord;

void main()
{
    gl_Position = iMatrix * vec4(iVertex, 1.0);
    vTexcoord = iCoords;
}