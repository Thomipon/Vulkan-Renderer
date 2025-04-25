module fresnel;

public float3 halfVector(float3 v1, float3 v2)
{
    return normalize(v1 + v2);
}

public float schlickFresnel(float3 view, float3 h)
{
    float base = 1.f - saturate(dot(view, h));
    float baseSq = base * base;
    return baseSq * baseSq * base;
}