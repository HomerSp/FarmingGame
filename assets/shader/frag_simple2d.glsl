#version 130

uniform sampler2D texture;
varying vec2 v_texcoord;

void main(void)
{
    gl_FragColor = texture2D(texture, v_texcoord);
};