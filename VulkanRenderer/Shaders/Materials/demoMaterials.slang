module demoMaterials;

import Core.material;
import Core.geometry;
import BRDF.pbr;
import BRDF.combinedBRDFs;

import Library.common;

struct HorizontalBlendDemo : IMaterial
{
    typedef HorizontalBlendBRDF<PBRBRDF, PBRBRDF> BRDF

    float3 albedo1;
    float metallic1;
    float roughness1;

    float3 albedo2;
    float metallic2;
    float roughness2;

    float blendScale;

    MaterialResult<BRDF> evaluate(SurfaceGeometry geometry)
    {
        BRDF combined = {};
        combined.brdf1 = getBRDF(albedo1, geometry.worldNormal, metallic1, roughness1);
        combined.brdf2 = getBRDF(albedo2, geometry.worldNormal, metallic2, roughness2);
        combined.lerpFactor = cubicInterpolationFactor(geometry.getLocalPosition().x * blendScale + .5f);
        return {combined, geometry};
    }

    PBRBRDF getBRDF(float3 albedo, float3 normal, float metallic, float roughness)
    {
        PBRBRDF brdf = {};
        brdf.diffuse.albedo = lerp(albedo, 0.f, metallic);
        brdf.normal = normal;
        brdf.fresnel.f0 = lerp(0.04f, albedo, metallic);
        brdf.fresnel.f90 = float3(1.f);
        brdf.roughness = roughness;
        brdf.emissive = float3(0.f);
        brdf.ambientOcclusion = 1.f;
        return brdf;
    }
}