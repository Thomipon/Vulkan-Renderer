module combinedBRDFs;

import Core.brdf;
import Core.indirectLighting;
import BRDF.topLayerBSDF;

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

public struct VerticalBlendBRDF<BottomBRDF : IBRDF, TopBSDF : ITopLayerBSDF> : IBRDF
{
    BottomBRDF bottomBrdf;
    TopBSDF topBsdf;

    public float3 evaluate(float3 viewDirection, float3 lightDirection, float3 lightColor)
    {
        float3 reflected;
        float3 transmittedDirection;
        float3 transmittedLight = topBsdf.transmitForward(lightDirection, viewDirection, lightColor, reflected, transmittedDirection);

        float3 refractedView;
        float3 internalReflectionFactor;
        float3 backTransmittanceFactor = topBsdf.transmitBackwards(viewDirection, refractedView, internalReflectionFactor);
        float3 bottomLight = bottomBrdf.evaluate(refractedView, transmittedDirection, transmittedLight);
        float3 firstBounceExitLight = bottomLight * backTransmittanceFactor;
        float3 firstInternalBounceLight = bottomLight * internalReflectionFactor;

        float3 relativeInternalReflection = firstInternalBounceLight / transmittedLight;
        float3 internalReflectionGeometricSeries = 1.f / (1.f - relativeInternalReflection);

        return reflected + firstBounceExitLight * internalReflectionGeometricSeries;
    }

    public float3 evaluateIndirect<Environment : IIndirectLightEnvironment>(float3 viewDirection, Environment environment)
    {
        // TODO
        return 0.f;
    }

    public float3 evaluateEmissive(float3 viewDirection)
    {
        float3 refractedView;
        float3 internalReflectionFactor;
        float3 backTransmittanceFactor = topBsdf.transmitBackwards(viewDirection, refractedView, internalReflectionFactor);

        float3 internalReflectionGeometricSeries = 1.f / (1.f - internalReflectionFactor);
        return bottomBrdf.evaluateEmissive(refractedView) * internalReflectionGeometricSeries * internalReflectionFactor;
    }
}
