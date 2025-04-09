module phong;

import Core.brdf;
import Core.material;
import Core.geometry;

struct PhongBRDF : IBRDF
{
    float3 kd;
    float3 ks;
    float specularity;
    float3 normal;

    float3 evaluate(float3 viewDirection, float3 lightDirection, float3 lightColor)
    {
        float nDotL = saturate(dot(normal, lightDirection));
        float3 H = normalize(lightDirection + viewDirection);
        float nDotH = saturate(dot(normal, H));

        return lightColor * (kd * nDotL + ks * pow(nDotH, specularity));
    }
}

struct ConstantPhongMaterial : IMaterial
{
    float3 diffuseColor;
    float3 specularColor;
    float specularity;

    // To satisfy the requirements of the `IMaterial` interface, our
    // material type needs to provide a suitable `BRDF` type. We
    // do this by using a simple `typedef`, although a nested
    // `struct` type can also satisfy an associated type requirement.
    //
    // A future version of the Slang compiler may allow the "right"
    // associated type definition to be inferred from the signature
    // of the `prepare()` method below.
    //
    typedef PhongBRDF BRDF;

    MaterialResult<PhongBRDF> evaluate(SurfaceGeometry geometry)
    {
        PhongBRDF brdf;
        brdf.kd = diffuseColor;
        brdf.ks = specularColor;
        brdf.specularity = specularity;
        brdf.normal = geometry.worldNormal;
        return {brdf, geometry};
    }
}