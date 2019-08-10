#version 330 core

out vec4 oColor;

uniform vec4 iInnerColor;
uniform vec4 iOuterColor;
uniform float iMod;

varying vec2 vTexcoord;

void main(void)
{
    float d = dot(vTexcoord, vTexcoord);
    if (d > 1.0) {
        discard;
    }

    vec3 c = mix(iInnerColor.rgb, iOuterColor.rgb, d);
    float a = (1.0 - smoothstep(0.0, 1.0, d));
    float m = iMod * 0.07;
    oColor = vec4(c, a - m);
};