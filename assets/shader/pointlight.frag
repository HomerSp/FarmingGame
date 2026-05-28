#version 450

layout(std140, set = 3, binding = 0) uniform FragmentUniforms {
    float iMod;
};

layout(location = 0) out vec4 oColor;

layout(location = 0) in vec2 vCoords;
layout(location = 1) in vec4 vInnerColor;
layout(location = 2) in vec4 vOuterColor;

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
}
