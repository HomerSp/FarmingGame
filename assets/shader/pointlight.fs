#version 330 core

uniform float iMod;

out vec4 oColor;

in vec2 vCoords;
in vec4 vInnerColor;
in vec4 vOuterColor;

void main(void)
{
    float d = dot(vCoords, vCoords);
    if (d > 1.0) {
        discard;
    }

    vec3 c = mix(vInnerColor.rgb, vOuterColor.rgb, d);
    float a = (1.0 - smoothstep(0.0, 1.0, d));
    float m = iMod * 0.07;
    oColor = vec4(c, a - m);
};