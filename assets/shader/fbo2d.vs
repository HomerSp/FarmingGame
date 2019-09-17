#version 330 core

uniform mat4 uWorldMatrix;

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec4 iPosSize;

out vec2 vTexCoords;
out float vTexLayer;

void main()
{
    vec2 pos = iPosSize.xy + iVertex * iPosSize.zw;
    gl_Position = uWorldMatrix * vec4(pos, 0.0, 1.0);

    vTexCoords = iPosSize.xy + (iVertex * iPosSize.zw);
    vTexCoords.y = iPosSize.w - vTexCoords.y;
}