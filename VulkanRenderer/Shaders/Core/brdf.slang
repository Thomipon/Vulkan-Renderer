module brdf;

import indirectLighting;

public interface IBRDF
{
    float3 evaluate(float3 viewDirection, float3 lightDirection, float3 lightColor);
    float3 evaluateIndirect<Environment : IIndirectLightEnvironment>(float3 viewDirection, float3 lightColor, Environment environment);
}