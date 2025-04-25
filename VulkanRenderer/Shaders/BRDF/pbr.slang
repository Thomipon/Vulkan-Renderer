module pbr;

import Core.brdf;
import Core.material;
import Core.geometry;
import Core.indirectLighting;

import Library.fresnel;

struct PBRBRDF : IBRDF
{
    float3 albedo;
    float3 f0;
    float3 f90;
    float roughness;
    float3 normal;
    float ambientOcclusion;

    float occlusionGGX(float3 direction, float alpha)
    {
        float dotNV = saturate(dot(normal, direction));
        float alphaSq = alpha * alpha;
        float rootTerm = sqrt(alphaSq + (1.f - alphaSq) * dotNV * dotNV);
        return 2.f * dotNV / (dotNV + rootTerm);
    }

    float geometryGGX(float3 view, float3 light, float alpha)
    {
        return occlusionGGX(view, alpha) * occlusionGGX(light, alpha);
    }

    float distributionGGX(float3 view, float3 light, float alpha)
    {
        float3 h = halfVector(view, light);
        float alphaSq = alpha * alpha;
        float dotNH = saturate(dot(normal, h));
        float denominator = dotNH * dotNH * (alphaSq - 1.f) + 1.f;
        return alphaSq / (denominator * denominator * float.getPi());
    }

    float3 cookTorranceGGX(float3 view, float3 light)
    {
        float alpha = roughness * roughness;
        float g = saturate(geometryGGX(view, light, alpha));
        float d = max(distributionGGX(view, light, alpha), 0.f);
        return g * d / (saturate(dot(normal, view)) * 4.f); // / dot(normal, light) <- not needed because we multiply by it anyways later
    }

    float3 lambertDiffuse()
    {
        return albedo / float.getPi();
    }

    float3 evaluate(float3 viewDirection, float3 lightDirection, float3 lightColor)
    {
        float3 h = halfVector(viewDirection, lightDirection);
        float fresnel = schlickFresnel(viewDirection, h);
        float3 f = lerp(f0, f90, fresnel);
        float3 diffuse = saturate(dot(normal, lightDirection)) * lambertDiffuse();
        float3 specular = cookTorranceGGX(viewDirection, lightDirection);
        return lerp(diffuse, specular, f) * lightColor;
    }

    float3 evaluateIndirect<Environment : IIndirectLightEnvironment>(float3 viewDirection, float3 lightColor, Environment environment)
    {
        float fresnel = schlickFresnel(viewDirection, normal);
        float3 f = lerp(f0, f90, fresnel);
        float3 diffuse = lambertDiffuse() * lightColor;
        float3 specular = lightColor;
        return lerp(diffuse, specular, f) * ambientOcclusion;
    }
}

struct ConstantPBRMaterial : IMaterial
{
    typedef PBRBRDF BRDF;

    float3 albedo;
    float3 f0;
    float3 f90;
    float roughness;

    MaterialResult<PBRBRDF> evaluate(SurfaceGeometry geometry)
    {
        PBRBRDF brdf = {};
        brdf.albedo = albedo;
        brdf.normal = geometry.worldNormal;
        brdf.f0 = f0;
        brdf.f90 = f90;
        brdf.roughness = roughness;
        brdf.ambientOcclusion = 1.f;
        return {brdf, geometry};
    }
}
