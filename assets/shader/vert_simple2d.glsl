#version 330 core

uniform mat4 iMatrix;

layout(location = 0) in vec2 iCoords;
layout(location = 1) in vec3 iVertex;
out vec2 vCoords;

void main()
{
    gl_Position = iMatrix * vec4(iVertex, 1.0);
    vCoords = iCoords;
}