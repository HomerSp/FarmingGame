#version 330 core

uniform mat4 iWorldMatrix;
uniform mat4 iProjectionMatrix;

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec4 iColor;
layout(location = 2) in vec2 iResolution;

out vec2 vCoords;
out vec4 vColor;

void main()
{
    gl_Position = iWorldMatrix * iProjectionMatrix * vec4(iVertex.xy / iResolution.xy, 0.0, 1.0);
    vCoords = iVertex.xy / iResolution.xy;
    vColor = iColor;
}