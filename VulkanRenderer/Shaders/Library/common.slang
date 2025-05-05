module common;

public float3 halfVector(float3 v1, float3 v2)
{
    return normalize(v1 + v2);
}