#version 130

attribute vec3 iVertex;
attribute vec2 iTexcoord;
uniform mat4 iMatrix;

varying vec2 vTexcoord;

void main()
{
    gl_Position = iMatrix * vec4(iVertex, 1.0);
    vTexcoord = iTexcoord;
}