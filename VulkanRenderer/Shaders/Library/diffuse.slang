module diffuse;

import Library.common;
import Library.fresnel;

public interface IDiffuseTerm
{
    float3 evaluateDiffuseTerm(float3 normal, float3 view, float3 light, float3 fresnel, float alpha);
    float3 evaluateUnattenuatedDiffuseTerm(float3 normal, float3 view, float3 fresnel, float alpha);
}

public struct LambertDiffuse : IDiffuseTerm
{
    public float3 albedo;

    public float3 evaluateDiffuseTerm(float3 normal, float3 view, float3 light, float3 fresnel, float alpha)
    {
        return saturate(dot(normal, light)) * evaluateUnattenuatedDiffuseTerm(normal, view, fresnel, alpha);
    }

    public float3 evaluateUnattenuatedDiffuseTerm(float3 normal, float3 view, float3 fresnel, float alpha)
    {
        return albedo * (1.f - fresnel) / float.getPi();
    }
}

public struct BurleyDiffuse<FresnelApproximation : IFresnelApproximation> : IDiffuseTerm
{
    public float3 albedo;

    private float f90FromRoughness(float3 view, float3 light, float alpha)
    {
        float3 h = halfVector(view, light);
        float cosd = saturate(dot(h, view));
        return f90FromRoughnessAndCos(alpha, cosd);
    }

    private float f90FromRoughnessAndCos(float alpha, float cosd)
    {
        return .5f + 2.f * alpha * cosd * cosd;
    }

    public float3 evaluateDiffuseTerm(float3 normal, float3 view, float3 light, float3 fresnel, float alpha)
    {
        float fl = FresnelApproximation.approximateFresnel(light, normal);
        float fv = FresnelApproximation.approximateFresnel(view, normal);
        float f90 = f90FromRoughness(view, light, alpha);
        float burleyFactor = lerp(1.f, f90, fl) * lerp(1.f, f90, fv);
        return burleyFactor * saturate(dot(normal, light)) * albedo * (1.f - fresnel) / float.getPi();
    }

    public float3 evaluateUnattenuatedDiffuseTerm(float3 normal, float3 view, float3 fresnel, float alpha)
    {
        float fv = FresnelApproximation.approximateFresnel(view, normal);
        float f90 = f90FromRoughnessAndCos(alpha, saturate(dot(view, normal)));
        float burleyFactor = lerp(1.f, f90, fv);
        return burleyFactor * albedo * (1.f - fresnel) / float.getPi();
    }
}

public typedef BurleyDiffuse<SchlickFresnel> DefaultDiffuseTerm;
