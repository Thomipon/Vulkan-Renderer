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