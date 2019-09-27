#version 330 core

uniform bool uRound = false;

out vec4 oColor;

in vec4 vColor;
in vec2 vCoords;

void main(void)
{
    if (uRound) {
        float d = dot(vCoords, vCoords);
        if (d > 1.0) {
            discard;
        }

        float a = (1.0 - smoothstep(0.0, 1.0, d));
        oColor = vec4(vColor.rgb, a * vColor.a);
    } else {
        oColor = vColor;
    }
};