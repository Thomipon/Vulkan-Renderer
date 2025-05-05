module specular;

import Library.common;

interface ISpecularDistributionTerm
{
    float evaluateDistributionTerm(float3 normal, float3 view, float3 light, float alpha);
}

public struct GGXIsotropicDistributionTerm : ISpecularDistributionTerm
{
    public float evaluateDistributionTerm(float3 normal, float3 view, float3 light, float alpha)
    {
        float3 h = halfVector(view, light);
        float alphaSq = alpha * alpha;
        float dotNH = saturate(dot(normal, h));
        float denominator = dotNH * dotNH * (alphaSq - 1.f) + 1.f;
        return alphaSq / (denominator * denominator * float.getPi());
    }
}

interface ISpecularGeometryTerm
{
    float evaluateGeometryTerm(float3 normal, float3 view, float3 light, float alpha);
}

public struct GGXGeometryTerm : ISpecularGeometryTerm
{
    private float occlusion(float3 normal, float3 direction, float alpha)
    {
        float dotNV = saturate(dot(normal, direction));
        float alphaSq = alpha * alpha;
        float rootTerm = sqrt(alphaSq + (1.f - alphaSq) * dotNV * dotNV);
        return 2.f * dotNV / (dotNV + rootTerm);
    }

    public float evaluateGeometryTerm(float3 normal, float3 view, float3 light, float alpha)
    {
        return occlusion(normal, view, alpha) * occlusion(normal, light, alpha);
    }
}

interface ISpecularTerm
{
    float3 evaluateSpecularTerm(float3 normal, float3 view, float3 light, float3 fresnel, float alpha);
}

public struct CookTorranceSpecularTerm<Distribution : ISpecularDistributionTerm, Geometry : ISpecularGeometryTerm> : ISpecularTerm
{
    public Distribution distribution;
    public Geometry geometry;

    public float3 evaluateSpecularTerm(float3 normal, float3 view, float3 light, float3 fresnel, float alpha)
    {
        float g = saturate(geometry.evaluateGeometryTerm(normal, view, light, alpha));
        float d = max(distribution.evaluateDistributionTerm(normal, view, light, alpha), 0.f);
        return fresnel * g * d / (saturate(dot(normal, view)) * 4.f); // / dot(normal, light) <- not needed because we multiply by it anyways later
    }
}
