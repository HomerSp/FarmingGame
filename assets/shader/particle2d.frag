#version 450

layout(std140, set = 3, binding = 0) uniform FragmentUniforms {
    int uRound;
};

layout(location = 0) out vec4 oColor;

layout(location = 0) in vec2 vCoords;
layout(location = 1) in vec4 vColor;

void main(void)
{
    if (uRound != 0) {
        float d = dot(vCoords, vCoords);
        if (d > 1.0) {
            discard;
        }

        float a = (1.0 - smoothstep(0.0, 1.0, d));
        oColor = vec4(vColor.rgb, a * vColor.a);
    } else {
        oColor = vColor;
    }
}
