module basicMaterials;

import Core.material;
import Core.geometry;

import BRDF.basicBRDFS;

struct SkySphereMaterial : IMaterial
{
    SamplerCube cubemap;
    float emissiveIntensity;

    typedef UnlitBRDF BRDF;

    MaterialResult<UnlitBRDF> evaluate(SurfaceGeometry geometry)
    {
        UnlitBRDF brdf;
        brdf.emissive = cubemap.Sample(geometry.worldNormal).rgb * emissiveIntensity;
        return {brdf, geometry};
    }
}