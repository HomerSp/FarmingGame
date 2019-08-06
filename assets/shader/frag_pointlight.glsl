#version 130

uniform vec4 iColor;
uniform float iMod;

varying vec2 vTexcoord;

void main(void)
{
    float d = dot(vTexcoord, vTexcoord);
    if (d > 1.0) {
        discard;
    }

    float a = (1.0 - smoothstep(0.0, 1.0, d));
    float m = iMod * 0.03;
    gl_FragColor = vec4(iColor.rgb, (a * iColor.a) - m);
};