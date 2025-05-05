module diffuse;

import Library.fresnel;

interface IDiffuseTerm
{
    float3 evaluateDiffuseTerm(float3 normal, float3 view, float3 light, float3 fresnel, float alpha);
}

public struct LambertDiffuse : IDiffuseTerm
{
    public float3 albedo;

    public float3 evaluateDiffuseTerm(float3 normal, float3 view, float3 light, float3 fresnel, float alpha)
    {
        return saturate(dot(normal, light)) * albedo * (1.f - fresnel) / float.getPi();
    }
}

public struct BurleyDiffuse<FresnelApproximation : IFresnelApproximation> : IDiffuseTerm
{
    public float3 evaluateDiffuseTerm(float3 normal, float3 view, float3 light, float3 fresnel, float alpha)
    {

    }
}
