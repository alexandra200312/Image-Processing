#version 410

layout(location = 0) in vec2 texture_coord;
layout(location = 0) out vec4 out_color;

uniform sampler2D textureImage;
uniform ivec2 screenSize;

uniform vec2 mousePos;
uniform float focusRadius;
uniform float maxBlurRadius;

uniform int outputMode;
uniform int flipVertical;

vec2 texCoord()
{
    vec2 tc = texture_coord;
    if (flipVertical == 1)
        tc.y = 1.0 - tc.y;
    return tc;
}

float gaussian(float x, float sigma)
{
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

float computeBlurAmount()
{
    float dist = distance(gl_FragCoord.xy, mousePos);

    if (dist < focusRadius)
        return 0.0;

    float norm = (dist - focusRadius) / (focusRadius * 2.0);
    return smoothstep(0.0, 1.0, clamp(norm, 0.0, 1.0));
}

vec4 blurHorizontal(float blurAmount)
{
    vec2 texel = 1.0 / vec2(screenSize);
    int radius = int(maxBlurRadius * blurAmount);
    float sigma = maxBlurRadius * blurAmount * 0.8 + 0.5;

    vec4 sum = vec4(0.0);
    float weightSum = 0.0;

    for (int x = -radius; x <= radius; x++)
    {
        float w = gaussian(float(x), sigma);
        sum += texture(textureImage, texCoord() + vec2(x, 0) * texel) * w;
        weightSum += w;
    }

    return sum / weightSum;
}

vec4 blurVertical(float blurAmount)
{
    vec2 texel = 1.0 / vec2(screenSize);
    int radius = int(maxBlurRadius * blurAmount * 0.6);
    float sigma = maxBlurRadius * blurAmount * 0.8 + 0.5;

    vec4 sum = vec4(0.0);
    float weightSum = 0.0;

    for (int y = -radius; y <= radius; y++)
    {
        float w = gaussian(float(y), sigma);
        sum += texture(textureImage, texCoord() + vec2(0, y) * texel) * w;
        weightSum += w;
    }

    return sum / weightSum;
}

bool isOnCircle(float dist, float radius)
{
    float thickness = 1.5;
    return abs(dist - radius) < thickness;
}

void main()
{
    vec4 original = texture(textureImage, texCoord());
    float blurAmount = computeBlurAmount();

    if (outputMode == 0)
    {
        out_color = original;
    }
    else if (outputMode == 1)
    {
        out_color = blurHorizontal(blurAmount);
    }
    else if (outputMode == 2)
    {
        out_color = blurVertical(blurAmount);
    }
    else if (outputMode == 3)
    {
        vec4 blurred = blurVertical(blurAmount);
        out_color = mix(original, blurred, blurAmount);
    }
    else if (outputMode == 4)
    {
        vec4 blurred = blurVertical(blurAmount);
        vec4 finalColor = mix(original, blurred, blurAmount);

        float dist = distance(gl_FragCoord.xy, mousePos);

        bool circle =
            isOnCircle(dist, focusRadius) ||
            isOnCircle(dist, focusRadius * 0.5) ||
            isOnCircle(dist, focusRadius * 0.25);

        if (circle)
            out_color = mix(finalColor, vec4(1.0), 0.7);
        else
            out_color = finalColor;
    }
}
