module brdf;

public interface IBRDF
{
    float3 evaluate(float3 viewDirection, float3 lightDirection, float3 lightColor);
    float3 evaluateIndirect(float3 viewDirection, float3 lightColor);
}