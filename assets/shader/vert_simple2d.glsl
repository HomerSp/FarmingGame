#version 130

attribute highp vec3 vertex;
attribute highp vec2 a_texcoord;
uniform highp mat4 matrix;

varying vec2 v_texcoord;

void main()
{
    gl_Position = matrix * vec4(vertex, 1.0);
    v_texcoord = a_texcoord;
}