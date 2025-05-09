module demoMaterials;

import Core.material;
import Core.geometry;
import BRDF.pbr;
import BRDF.combinedBRDFs;
import BRDF.topLayerBSDF;

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

struct VerticalLayerDemo : IMaterial
{
    typedef VerticalBlendBRDF<PBRBRDF, DefaultTopLayerBSDF> BRDF;

    float3 bottomAlbedo;
    float bottomMetallic;
    float bottomRoughness;
    float3 bottomEmissive;

    float topCoverage;
    float topThickness;
    float topRoughness;
    float3 topTransmittance;
    float topIor;
    float3 topF0;

    MaterialResult<BRDF> evaluate(SurfaceGeometry geometry)
    {
        BRDF layered = {};

        layered.bottomBrdf.diffuse.albedo = lerp(bottomAlbedo, 0.f, bottomMetallic);
        layered.bottomBrdf.normal = geometry.worldNormal;
        layered.bottomBrdf.fresnel.f0 = lerp(0.04f, bottomAlbedo, bottomMetallic);
        layered.bottomBrdf.fresnel.f90 = float3(1.f);
        layered.bottomBrdf.roughness = bottomRoughness;
        layered.bottomBrdf.emissive = bottomEmissive;
        layered.bottomBrdf.ambientOcclusion = 1.f;

        layered.topBsdf.normal = geometry.worldNormal;
        layered.topBsdf.coverage = topCoverage;
        layered.topBsdf.thickness = topThickness;
        layered.topBsdf.roughness = topRoughness;
        layered.topBsdf.transmittance = topTransmittance;
        layered.topBsdf.ior = topIor;
        layered.topBsdf.fresnel.f0 = topF0;
        layered.topBsdf.fresnel.f90 = float3(1.f);
        layered.topBsdf.ambientOcclusion = float3(1.f);

        return {layered, geometry};
    }
}

struct Opal : IMaterial
{
    typedef VerticalBlendBRDF<PBRBRDF, DefaultTopLayerBSDF> BRDF;

    float textureTiling;
    Sampler2D normalMap;
    Sampler2D armMap;
    float hueShift;
    float hueScale;
    float saturation;
    float brightness;
    float specularIntensity;
    float3 bottomAlbedo;
    float roughnessThreshold;
    float roughnessCenter;
    float coatIOR;
    float coatInnerThickness;
    float coatOuterThickness;
    float coatRoughness;
    float3 coatTransmittance;
    float coatThicknessExponent;

    MaterialResult<BRDF> evaluate(SurfaceGeometry geometry)
    {
        PBRBRDF bottom = {};
        float2 uv = geometry.textureCoordinate * textureTiling;
        bottom.normal = normalize(mul(geometry.tangentToWorld, normalMap.Sample(uv).rgb));
        float3 viewDirection = normalize(geometry.viewData.viewPosition - geometry.worldPosition);
        float cosv = abs(dot(bottom.normal, viewDirection));
        float hue = abs(frac(cosv * hueScale + hueShift));
        float3 color = hsvToRgb(float3(hue, saturation, 1.f));
        bottom.diffuse.albedo = bottomAlbedo;
        float2 ar = armMap.Sample(uv).xy;
        bottom.ambientOcclusion = ar.x;
        bottom.roughness = remapDistribution(ar.y, roughnessCenter, roughnessThreshold);
        bottom.fresnel.f90 = float3(1.f);
        bottom.fresnel.f0 = color * specularIntensity;
        bottom.emissive = color * brightness;

        DefaultTopLayerBSDF top = {};
        top.normal = geometry.worldNormal;
        top.coverage = 1.f;
        top.thickness = lerp(coatInnerThickness, coatOuterThickness, pow(1.f - abs(dot(viewDirection, geometry.worldNormal)), coatThicknessExponent));
        top.roughness = coatRoughness;
        top.transmittance = coatTransmittance;
        top.ior = coatIOR;
        top.fresnel.f0 = f0FromIOR(coatIOR);
        top.fresnel.f90 = float3(1.f);
        top.ambientOcclusion = float3(1.f);

        return {{bottom, top}, geometry};
    }
}
