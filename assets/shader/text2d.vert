#version 450

layout(std140, set = 1, binding = 0) uniform VertexUniforms {
    mat4 uWorldMatrix;
};

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec4 iPosSize;    // dst: x, y, w, h
layout(location = 2) in vec4 iSrcRect;    // src: x, y, w, h

layout(location = 0) out vec2 vTexCoords;

void main()
{
    vec2 pos = iPosSize.xy + iVertex * iPosSize.zw;
    gl_Position = uWorldMatrix * vec4(pos, 0.0, 1.0);

    // UV coordinates: src offset + vertex * src size
    vTexCoords = iSrcRect.xy + iVertex * iSrcRect.zw;
}
