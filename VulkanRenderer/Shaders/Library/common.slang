module common;

public float3 halfVector(float3 v1, float3 v2)
{
    return normalize(v1 + v2);
}

public float cubicInterpolationFactor(float inAlpha)
{
    float x = saturate(inAlpha);
    float xsq = x * x;
    float xcb = xsq * x;
    return -2 * xcb + 3 * xsq;
}

public float3 hueToRgb(float hue)
{
    float r = min(hue, 1.f - hue);
    float div = .333333333f;
    float g = abs(hue - div);
    float b = abs(hue - (div + div));
    float3 rgb = saturate(div - float3(r, g, b));
    return rgb / (rgb.r + rgb.g + rgb.b);
}

public float3 hsvToRgb(float3 hsv)
{
    float3 hue = hueToRgb(hsv.x);
    return lerp(1.f, hue, hsv.y) * hsv.z;
}

public float f0FromIOR(float ior)
{
    float f0sqrt = (ior - 1.f) / (ior + 1.f);
    return f0sqrt * f0sqrt;
}
