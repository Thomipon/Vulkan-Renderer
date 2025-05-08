module fresnel;

import Library.common;

public interface IFresnelApproximation
{
    static float approximateFresnel(float3 view, float3 half);
}

public struct SchlickFresnel : IFresnelApproximation
{
    public static float approximateFresnel(float3 view, float3 half)
    {
        float base = 1.f - saturate(dot(view, half));
        float baseSq = base * base;
        return baseSq * baseSq * base;
    }
}

public interface IFresnel
{
    float3 getFresnelTerm(float3 view, float3 light);
    float3 getFresnelTermDirectly(float3 normal, float3 direction);
}

public struct F0F90Fresnel<FresnelApproximation : IFresnelApproximation> : IFresnel
{
    public float3 f0;
    public float3 f90;

    public float3 getFresnelTerm(float3 view, float3 light)
    {
        float3 half = halfVector(view, light);
        return getFresnelTermDirectly(half, light);
    }

    public float3 getFresnelTermDirectly(float3 normal, float3 direction)
    {
        float fresnel = FresnelApproximation.approximateFresnel(normal, direction);
        return lerp(f0, f90, fresnel);
    }
}

public typedef F0F90Fresnel<SchlickFresnel> DefaultFresnel;
