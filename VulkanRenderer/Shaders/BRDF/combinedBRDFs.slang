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
    public BottomBRDF bottomBrdf;
    public TopBSDF topBsdf;

    public float3 evaluate(float3 viewDirection, float3 lightDirection, float3 lightColor)
    {
        float3 reflected;
        float3 transmittedDirection;
        float3 surfacePenetratingLight;
        float3 transmittedLight = topBsdf.transmitForward(lightDirection, viewDirection, lightColor, reflected, transmittedDirection, surfacePenetratingLight);

        float3 refractedView;
        float3 internalReflectionFactor;
        float3 absorptionFactor;
        float3 backTransmittanceFactor = topBsdf.transmitBackwards(viewDirection, refractedView, internalReflectionFactor, absorptionFactor);
        float3 bottomLight = bottomBrdf.evaluate(refractedView, transmittedDirection, transmittedLight);
        float3 firstBounceExitLight = bottomLight * backTransmittanceFactor;
        float3 firstInternalBounceLight = bottomLight * internalReflectionFactor;

        float3 relativeInternalReflection = saturate(firstInternalBounceLight / surfacePenetratingLight);
        float3 internalReflectionGeometricSeries = 1.f / (1.f - relativeInternalReflection);

        return reflected + firstBounceExitLight * internalReflectionGeometricSeries;
    }

    public float3 evaluateIndirect<Environment : IIndirectLightEnvironment>(float3 viewDirection, Environment environment)
    {
        float3 indirectTop = topBsdf.evaluateIndirect(viewDirection, environment);

        float3 refractedView;
        float3 internalReflectionFactor;
        float3 absorptionFactor;
        float3 backTransmittanceFactor = topBsdf.transmitBackwards(viewDirection, refractedView, internalReflectionFactor, absorptionFactor);
        float3 internalReflectionGeometricSeries = 1.f / (1.f - internalReflectionFactor * absorptionFactor);

        let modifiedEnvironment = topBsdf.modifyIndirectLightEnvironment(environment);
        float3 indirectBottom = bottomBrdf.evaluateIndirect(refractedView, modifiedEnvironment) * backTransmittanceFactor * internalReflectionGeometricSeries;

        return indirectTop + indirectBottom;
    }

    public float3 evaluateEmissive(float3 viewDirection)
    {
        float3 refractedView;
        float3 internalReflectionFactor;
        float3 absorptionFactor;
        float3 backTransmittanceFactor = topBsdf.transmitBackwards(viewDirection, refractedView, internalReflectionFactor, absorptionFactor);

        // Internal reflection is ignored here. It would require evaluating the bottom BRDF
        return topBsdf.evaluateEmissive(viewDirection) + bottomBrdf.evaluateEmissive(refractedView) * backTransmittanceFactor;
    }
}
