#version 450

layout(std140, set = 1, binding = 0) uniform VertexUniforms {
    mat4 uWorldMatrix;
};

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec4 iInnerColor;
layout(location = 2) in vec4 iOuterColor;
layout(location = 3) in vec4 iPosSize;

layout(location = 0) out vec2 vCoords;
layout(location = 1) out vec4 vInnerColor;
layout(location = 2) out vec4 vOuterColor;

void main()
{
    vec2 pos = iPosSize.xy + iVertex * iPosSize.zw;
    gl_Position = uWorldMatrix * vec4(pos, 0.0, 1.0);
    vCoords = iVertex * 2.0 - 1.0;
    vInnerColor = iInnerColor;
    vOuterColor = iOuterColor;
}
