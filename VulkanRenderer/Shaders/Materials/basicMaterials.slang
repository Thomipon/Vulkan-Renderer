module basicMaterials;

import Core.material;
import Core.geometry;

import BRDF.basicBRDFS;

import Core.largeBlock;

struct SkySphereMaterial : IMaterial
{
    typedef UnlitBRDF BRDF;

    SamplerCube cubemap;
    float emissiveIntensity;

    LargeBlock _;

    MaterialResult<UnlitBRDF> evaluate(SurfaceGeometry geometry)
    {
        UnlitBRDF brdf = {};
        float3 environmentColor = cubemap.Sample(-geometry.worldNormal).rgb;
        brdf.emissive = environmentColor.rgb * emissiveIntensity;
        return {brdf, geometry};
    }
}
