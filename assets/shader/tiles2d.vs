#version 330 core

uniform mat4 uWorldMatrix;
uniform float uAnimFrame;

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec3 iPosition;
layout(location = 2) in vec4 iTexPosSize;
layout(location = 3) in vec2 iTexAnimSize;
layout(location = 4) in float iTexLayer;

out vec2 vTexCoords;
out float vTexLayer;

void main()
{
    vec2 pos = iPosition.xy + iVertex * iTexPosSize.zw;
    gl_Position = uWorldMatrix * vec4(pos, 0.0, 1.0);
    gl_Position.z = iPosition.z;

    vec2 texCoords = iTexPosSize.xy + (iVertex * iTexPosSize.zw);
    vTexCoords = texCoords + (iTexAnimSize * uAnimFrame);
    vTexLayer = iTexLayer;
}