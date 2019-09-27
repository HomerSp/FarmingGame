#version 330 core

uniform vec2 uCharSize;
uniform sampler2DArray uTexture;

out vec4 oColor;

flat in uint vChar;
in vec2 vTexCoords;
in float vTexLayer;

vec4 cubic(float v)
{
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2DArray sampler, ivec3 pixelCoords)
{
    vec2 texSize = textureSize(sampler, 0).xy;
    vec2 invTexSize = 1.0 / texSize;
    vec2 texCoords = pixelCoords.xy / texSize;

    texCoords = texCoords * texSize - 0.5;

    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;

    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;

    offset *= invTexSize.xxyy;

    vec4 sample0 = texture(sampler, vec3(offset.xz, pixelCoords.z));
    vec4 sample1 = texture(sampler, vec3(offset.yz, pixelCoords.z));
    vec4 sample2 = texture(sampler, vec3(offset.xw, pixelCoords.z));
    vec4 sample3 = texture(sampler, vec3(offset.yw, pixelCoords.z));

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(mix(sample3, sample2, sx), mix(sample1, sample0, sx), sy);
}

void main(void)
{
    vec2 colRow = vec2(mod(vChar, 10.0), floor(vChar / 10.0));
    vec2 texCoords = colRow * uCharSize + vTexCoords * uCharSize;

    vec4 c = textureBicubic(uTexture, ivec3(texCoords, vTexLayer));
    if (c.a == 0.0) {
        discard;
    }

    oColor = vec4(1, 1, 1, c.a);
};