module pbr;

import Core.brdf;
import Core.material;
import Core.geometry;
import Core.indirectLighting;

import Library.common;
import Library.fresnel;
import Library.diffuse;
import Library.specular;

struct PBRBRDF : IBRDF
{
    LambertDiffuse diffuse;
    F0F90Fresnel<SchlickFresnel> fresnel;
    CookTorranceSpecularTerm<GGXIsotropicDistributionTerm, GGXGeometryTerm> specular;
    float roughness;
    float3 normal;
    float3 emissive;
    float ambientOcclusion;

    float3 evaluate(float3 viewDirection, float3 lightDirection, float3 lightColor)
    {
        float3 f = fresnel.getFresnelTerm(viewDirection, lightDirection);
        float alpha = roughness * roughness;
        float3 d = diffuse.evaluateDiffuseTerm(normal, viewDirection, lightDirection, f, alpha);
        float3 s = specular.evaluateSpecularTerm(normal, viewDirection, lightDirection, f, alpha);
        return (s + d) * lightColor;
    }

    float3 evaluateIndirect<Environment : IIndirectLightEnvironment>(float3 viewDirection, Environment environment)
    {
        float3 f = fresnel.getFresnelTerm(viewDirection, normal);
        float alpha = roughness * roughness;
        // TODO: treat indirect differently
        float3 diffuse = diffuse.evaluateDiffuseTerm(normal, viewDirection, normal, f, alpha) * environment.sampleEnvironment(normal, 1.f);
        float3 r = -reflect(viewDirection, normal);
        // TODO: Only do the geometry here
        float3 s = specular.evaluateSpecularTerm(normal, viewDirection, r, f, alpha) * environment.sampleEnvironment(r, roughness);
        return (s + diffuse * (1.f - f)) * ambientOcclusion;
    }

    float3 evaluateEmissive(float3 viewDirection)
    {
        return max(emissive, 0.f);
    }
}

struct ConstantPBRMaterial : IMaterial
{
    typedef PBRBRDF BRDF;

    float3 albedo;
    float3 f0;
    float3 f90;
    float3 emissiveColor;
    float roughness;

    MaterialResult<PBRBRDF> evaluate(SurfaceGeometry geometry)
    {
        PBRBRDF brdf = {};
        brdf.diffuse.albedo = albedo;
        brdf.normal = geometry.worldNormal;
        brdf.fresnel.f0 = f0;
        brdf.fresnel.f90 = f90;
        brdf.roughness = roughness;
        brdf.emissive = emissiveColor;
        brdf.ambientOcclusion = 1.f;
        return {brdf, geometry};
    }
}
