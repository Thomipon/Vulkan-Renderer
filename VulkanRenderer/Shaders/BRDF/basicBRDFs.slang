module basicBRDFs;

import Core.brdf;
import Core.material;
import Core.geometry;
import Core.indirectLighting;

public struct UnlitBRDF : IBRDF
{
    public float3 emissive;

    public float3 evaluate(float3 viewDirection, float3 lightDirection, float3 lightColor)
    {
        return float3(0.f);
    }

    public float3 evaluateIndirect<Environment : IIndirectLightEnvironment>(float3 viewDirection, Environment environment)
    {
        return float3(0.f);
    }

    public float3 evaluateEmissive(float3 viewDirection)
    {
        return emissive;
    }
}

struct ConstantUnlitMaterial : IMaterial
{
    typedef UnlitBRDF BRDF;

    float3 emissive;

        float3 albedo;
        float3 f0;
        float3 f90;
        float3 emissiveColor;
        float roughness;

    MaterialResult<UnlitBRDF> evaluate(SurfaceGeometry geometry)
    {
        UnlitBRDF brdf;
        brdf.emissive = emissive;
        return {brdf, geometry};
    }
}