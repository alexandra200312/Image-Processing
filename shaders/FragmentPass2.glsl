#version 410

layout(location = 0) in vec2 texture_coord;
layout(location = 0) out vec4 out_color;

uniform sampler2D texBlurH;
uniform sampler2D texInfo;

uniform ivec2 screenSize;
uniform float maxBlurRadius;
uniform int doBlur;

vec2 texCoord()
{
    return texture_coord;
}

float gaussian(float x, float sigma)
{
    return exp(-(x*x) / (2.0*sigma*sigma));
}

vec4 blurVertical(float blurAmount)
{
    vec2 texel = 1.0 / vec2(screenSize);

    int radius = int(maxBlurRadius * blurAmount);
    radius = clamp(radius, 0, int(maxBlurRadius));

    if (radius == 0)
        return texture(texBlurH, texCoord());

    float sigma = max(0.5, maxBlurRadius * blurAmount * 0.8);

    vec4 sum = vec4(0.0);
    float wsum = 0.0;

    for (int y = -radius; y <= radius; y++)
    {
        float w = gaussian(float(y), sigma);
        sum += texture(texBlurH, texCoord() + vec2(0, y) * texel) * w;
        wsum += w;
    }

    return sum / wsum;
}

void main()
{
    vec2 uv = texCoord();
    vec4 info = texture(texInfo, uv);

    float blurAmount = (doBlur == 0) ? 0.0 : info.a;
    vec4 b1 = blurVertical(blurAmount * 0.6);
    vec4 b2 = blurVertical(blurAmount * 1.0);
    vec4 b3 = blurVertical(blurAmount * 1.4);

    out_color = b1 * 0.5 + b2 * 0.3 + b3 * 0.2;
}
