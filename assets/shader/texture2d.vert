#version 450

layout(std140, set = 1, binding = 0) uniform VertexUniforms {
    mat4 uWorldMatrix;
    int uReverseY;
};

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec4 iPosSize;
layout(location = 2) in vec4 iTexPosSize;
layout(location = 3) in float iTexLayer;

layout(location = 0) out vec2 vTexCoords;
layout(location = 1) out float vTexLayer;

void main()
{
    vec2 pos = iPosSize.xy + iVertex * iPosSize.zw;
    gl_Position = uWorldMatrix * vec4(pos, 0.0, 1.0);

    vTexCoords = iTexPosSize.xy + (iVertex * iTexPosSize.zw);
    if (uReverseY != 0) {
        vTexCoords.y = iTexPosSize.w - vTexCoords.y;
    }

    vTexLayer = iTexLayer;
}
