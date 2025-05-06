module topLayerBSDF;

import Library.fresnel;

interface ITopLayerBSDF
{
    float3 transmitForward(float3 light, float3 view, out float3 reflected);
    float3 transmitBackwards(float3 light, float3 view, out float3 reflected);
}

public struct BeerLambertTopLayerBSDF : ITopLayerBSDF
{
    public float3 normal;
    public float coverage;
    public float thickness;
    public float roughness;
    public float3 transmittance;

    public float3 transmitForward(float3 light, float3 view, float3 lightColor, out float3 reflected)
    {
        float cosl = saturate(dot(normal, light));
        float effectiveThickness = thickness / cosl;
        float3 fullCoverageTransmittedLight = lightColor * exp(-effectiveThickness * transmittance);
        float3 transmittedLight = lerp(lightColor, fullCoverageTransmittedLight, coverage);
    }

    public float3 transmitBackwards(float3 light, float3 view, out float3 reflected)
    {

    }
}