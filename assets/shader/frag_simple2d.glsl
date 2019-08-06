#version 130

uniform sampler2D iTexture;
varying vec2 vTexcoord;

void main(void)
{
    gl_FragColor = texture2D(iTexture, vTexcoord);
};