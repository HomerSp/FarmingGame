#version 330 core

uniform mat4 uWorldMatrix;

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec4 iColor;
layout(location = 2) in float iAlpha;
layout(location = 3) in vec2 iPos;
layout(location = 4) in vec2 iSize;
layout(location = 5) in vec2 iRotation;

out vec2 vCoords;
out vec4 vColor;

vec2 rotate(vec2 position, vec2 rotation, vec2 center)
{
    return vec2(
        center.x + ((position.x - center.x) * rotation.y + (position.y - center.y) * rotation.x),
        center.y + ((position.y - center.y) * rotation.y - (position.x - center.x) * rotation.x)
    );
}

void main()
{
    vec2 pos = rotate(iVertex * iSize, iRotation, iSize / 2) + iPos;
    gl_Position = uWorldMatrix * vec4(pos, 0.0, 1.0);
    vCoords = iVertex * 2 - 1;
    vColor = vec4(iColor.rgb, iAlpha);
}