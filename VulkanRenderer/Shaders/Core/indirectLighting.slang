module indirectLighting;

public interface IIndirectLightEnvironment
{
    float3 sampleEnvironment(float3 direction, float roughness);
}