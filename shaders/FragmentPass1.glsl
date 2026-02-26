#version 410

layout(location = 0) in vec2 texture_coord;

layout(location = 0) out vec4 outBlurH;
layout(location = 1) out vec4 outInfo;

uniform sampler2D textureImage;
uniform ivec2 screenSize;

uniform vec2 mousePos;
uniform float focusRadius;
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

float computeBlurAmount()
{
    if (doBlur == 0) return 0.0;

    vec2 fixedMouse = vec2(mousePos.x, screenSize.y - mousePos.y);
    float dist = distance(gl_FragCoord.xy, fixedMouse);

    if (dist < focusRadius) return 0.0;


    float norm = (dist - focusRadius) / focusRadius;
    norm = clamp(norm, 0.0, 1.0);
    return smoothstep(0.2, 1.0, norm);
}

vec4 blurHorizontal(float blurAmount)
{
    vec2 texel = 1.0 / vec2(screenSize);

    int radius = int(maxBlurRadius * blurAmount);
    radius = clamp(radius, 0, int(maxBlurRadius));

    if (radius == 0)
        return texture(textureImage, texCoord());

    float sigma = max(0.5, maxBlurRadius * blurAmount * 0.8);

    vec4 sum = vec4(0.0);
    float wsum = 0.0;

    for (int x = -radius; x <= radius; x++)
    {
        float w = gaussian(float(x), sigma);
        sum += texture(textureImage, texCoord() + vec2(x, 0) * texel) * w;
        wsum += w;
    }

    return sum / wsum;
}

void main()
{
    vec4 original = texture(textureImage, texCoord());
    float blurAmount = computeBlurAmount();

    outBlurH = blurHorizontal(blurAmount);
    outInfo  = vec4(original.rgb, blurAmount);
}
