#version 330 core

uniform sampler2D iTexture;
uniform vec2 iResolution;

out vec4 oColor;
in vec2 vCoords;

void main(void)
{
    vec2 c = vCoords.xy / iResolution.xy;
    oColor = texture2D(iTexture, c);
};