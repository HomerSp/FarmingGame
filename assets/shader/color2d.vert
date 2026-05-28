#version 450

layout(std140, set = 1, binding = 0) uniform VertexUniforms {
    mat4 uWorldMatrix;
};

layout(location = 0) in vec2 iVertex;
layout(location = 1) in vec4 iColor;
layout(location = 2) in mat4 iMatrix;

layout(location = 0) out vec2 vCoords;
layout(location = 1) out vec4 vColor;

void main()
{
    gl_Position = uWorldMatrix * iMatrix * vec4(iVertex, 0.0, 1.0);
    vCoords = iVertex;
    vColor = iColor;
}
