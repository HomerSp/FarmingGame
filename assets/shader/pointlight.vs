#version 330 core

uniform mat4 uWorldMatrix;

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec4 iInnerColor;
layout(location = 2) in vec4 iOuterColor;
layout(location = 3) in vec4 iPosSize;

out vec2 vCoords;
out vec4 vInnerColor;
out vec4 vOuterColor;

void main()
{
    vec2 pos = iPosSize.xy + iVertex * iPosSize.zw;
    gl_Position = uWorldMatrix * vec4(pos, 0.0, 1.0);
    vCoords = iVertex * 2 - 1;
    vInnerColor = iInnerColor;
    vOuterColor = iOuterColor;
}