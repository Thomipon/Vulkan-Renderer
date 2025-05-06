module combinedBRDFs;

import Core.brdf;
import Core.indirectLighting;

public struct HorizontalBlendBRDF<BRDF1 : IBRDF, BRDF2 : IBRDF> : IBRDF
{
    public BRDF1 brdf1;
    public BRDF2 brdf2;
    public float lerpFactor;

    public float3 evaluate(float3 viewDirection, float3 lightDirection, float3 lightColor)
    {
        return lerp(brdf1.evaluate(viewDirection, lightDirection, lightColor), brdf2.evaluate(viewDirection, lightDirection, lightColor), saturate(lerpFactor));
    }

    public float3 evaluateIndirect<Environment : IIndirectLightEnvironment>(float3 viewDirection, Environment environment)
    {
        return lerp(brdf1.evaluateIndirect(viewDirection, environment), brdf2.evaluateIndirect(viewDirection, environment), saturate(lerpFactor));
    }

    public float3 evaluateEmissive(float3 viewDirection)
    {
        return lerp(brdf1.evaluateEmissive(viewDirection), brdf2.evaluateEmissive(viewDirection), saturate(lerpFactor));
    }
}

public struct VerticalBlendBRDF<BottomBRDF : IBRDF> : IBRDF
{
    BottomBRDF bottomBrdf;

    float topLayerCoverage;
    float topLayerThickness;
    float topLayerRoughness;
    float topLayerTransmittance;

    public float3 evaluate(float3 viewDirection, float3 lightDirection, float3 lightColor)
    {
        return lerp(brdf1.evaluate(viewDirection, lightDirection, lightColor), brdf2.evaluate(viewDirection, lightDirection, lightColor), saturate(lerpFactor));
    }

    public float3 evaluateIndirect<Environment : IIndirectLightEnvironment>(float3 viewDirection, Environment environment)
    {
        return lerp(brdf1.evaluateIndirect(viewDirection, environment), brdf2.evaluateIndirect(viewDirection, environment), saturate(lerpFactor));
    }

    public float3 evaluateEmissive(float3 viewDirection)
    {
        return lerp(brdf1.evaluateEmissive(viewDirection), brdf2.evaluateEmissive(viewDirection), saturate(lerpFactor));
    }
}
