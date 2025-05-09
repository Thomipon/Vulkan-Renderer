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

public float remapValueRange(float inValue, float outValueMin, float outValueMax, float inValueMin, float inValueMax, bool clampResult = true)
{
    float alpha = (inValue - inValueMin) / (inValueMax - inValueMin);
    float result = lerp(outValueMin, outValueMax, alpha);
    if (clampResult)
    {
        return clamp(result, outValueMin, outValueMax);
    }
    return result;
}

public float remapDistribution(float inValue, float outValueCenter, float inValueThreshold = .5, bool clampResult = true, float outValueMin = 0., float outValueMax = 1., float inValueMin = 0., float inValueMax = 1.)
{
    float lowRange = remapValueRange(inValue, outValueMin, outValueCenter, inValueMin, inValueThreshold, clampResult);
    float highRange = remapValueRange(inValue, outValueCenter, outValueMax, inValueThreshold, inValueMax, clampResult);
    return inValue < inValueThreshold ? lowRange : highRange;
}
