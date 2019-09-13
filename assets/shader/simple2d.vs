#version 330 core

uniform mat4 uWorldMatrix;

layout(location = 0) in vec2 iCoords;
layout(location = 1) in vec2 iVertex;
layout(location = 2) in mat4 iMatrix;

out vec2 vCoords;

void main()
{
    gl_Position = uWorldMatrix * iMatrix * vec4(iVertex, 0.0, 1.0);
    vCoords = iCoords;
}