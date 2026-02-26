#version 410

layout(location = 0) in vec2 texture_coord;
layout(location = 0) out vec4 out_color;

uniform sampler2D texOriginal;
uniform sampler2D texPass1BlurH;
uniform sampler2D texPass1Info;
uniform sampler2D texFinal;

uniform int outputMode;
uniform int flipVertical;

uniform vec2 mousePos;
uniform float focusRadius;
uniform ivec2 screenSize;

vec2 texCoord()
{
    vec2 tc = texture_coord;
    if (flipVertical == 1) tc.y = 1.0 - tc.y;
    return tc;
}

bool isOnCircle(float dist, float radius)
{
    float thickness = 1.5;
    return abs(dist - radius) < thickness;
}

void main()
{
    vec2 uv = texCoord();

    vec4 original = texture(texOriginal, uv);
    vec4 pass1H   = texture(texPass1BlurH, uv);
    vec4 info     = texture(texPass1Info, uv);
    vec4 finalImg = texture(texFinal, uv);

    float blurAmount = info.a;

    vec4 base;
    if (outputMode == 0) base = original;
    else if (outputMode == 1) base = pass1H;
    else if (outputMode == 2) base = finalImg;
    else if (outputMode == 3) base = finalImg;
    else base = finalImg;

    if (outputMode == 4)
    {
        vec2 fixedMouse = mousePos;

    if (flipVertical == 1)
        fixedMouse.y = screenSize.y - mousePos.y;

    vec2 pixelPos = vec2(uv.x * screenSize.x, uv.y * screenSize.y);
    float dist = distance(pixelPos, fixedMouse);

    bool circle = isOnCircle(dist, focusRadius) ||
                  isOnCircle(dist, focusRadius * 0.5) ||
                  isOnCircle(dist, focusRadius * 0.25);

    if (circle)
        base = mix(base, vec4(1.0), 0.7);
    }

    out_color = base;
}
