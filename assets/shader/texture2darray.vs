#version 330 core

uniform mat4 iWorldMatrix;
uniform mat4 iProjectionMatrix;

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec2 iTexPos;
layout(location = 2) in vec2 iTexSize;
layout(location = 3) in float iLayer;
layout(location = 4) in mat4 iMatrix;

out vec2 vCoords;
out float vLayer;

void main()
{
    gl_Position = iWorldMatrix * iProjectionMatrix * iMatrix * vec4(iVertex, 0.0, 1.0);
    vCoords = iTexPos.xy + (iVertex.xy * iTexSize.xy);
    vLayer = iLayer;
}