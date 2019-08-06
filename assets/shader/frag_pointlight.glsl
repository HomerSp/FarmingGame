#version 130

uniform sampler2D texture;
uniform vec4 color;
uniform float mod;

varying vec2 v_texcoord;

void main(void)
{
    vec4 texc = texture2D(texture, v_texcoord);
    float m = mod * 0.03;
    gl_FragColor = vec4(color.rgb, (texc.a * color.a) - m);
};