#version 330 core

uniform mat4 uWorldMatrix;
uniform bool uReverseY = false;

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec4 iPosSize;
layout(location = 2) in vec4 iTexPosSize;

out vec2 vTexCoords;

void main()
{
    vec2 pos = iPosSize.xy + iVertex * iPosSize.zw;
    gl_Position = uWorldMatrix * vec4(pos, 0.0, 1.0);

    vTexCoords = iTexPosSize.xy + (iVertex * iTexPosSize.zw);
    if (uReverseY) {
        vTexCoords.y = iTexPosSize.w - vTexCoords.y;
    }
}