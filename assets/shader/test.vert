#version 450

layout(std140, set = 1, binding = 0) uniform VertexUniforms {
    mat4 uWorldMatrix;
    vec2 uCharSize;
};

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec3 iPosSize;
layout(location = 2) in uint iChar;

layout(location = 0) flat out uint vChar;
layout(location = 1) out vec2 vTexCoords;
layout(location = 2) out float vTexLayer;

void main()
{
    float ratio = uCharSize.x / uCharSize.y;
    vec2 pos = iPosSize.xy + iVertex * vec2(iPosSize.z * ratio, iPosSize.z);
    gl_Position = uWorldMatrix * vec4(pos, 0.0, 1.0);

    vChar = iChar - uint(32);
    vTexCoords = iVertex;
    vTexLayer = 0.0;
}
