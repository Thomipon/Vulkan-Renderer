module topLayerBSDF;

import Library.fresnel;
import Library.specular;

public interface ITopLayerBSDF
{
    float3 transmitForward(float3 light, float3 view, float3 lightColor, out float3 reflected, out float3 transmittedDirection);
    float3 transmitBackwards(float3 view, out float3 refractedView, out float3 internalReflectionColor);
    float3 evaluateEmissive(float3 view);
}

public struct BeerLambertTopLayerBSDF<Fresnel : IFresnel, SpecularTerm : ISpecularTerm> : ITopLayerBSDF
{
    public float3 normal;
    public float coverage;
    public float thickness;
    public float roughness;
    public float3 transmittance;
    public float ior;
    public Fresnel fresnel;
    public SpecularTerm specular;
    public float3 emissive;

    public float3 transmitForward(float3 light, float3 view, float3 lightColor, out float3 reflected, out float3 transmittedDirection)
    {
        transmittedDirection = -refract(-light, normal, ior);
        float cosl = saturate(dot(normal, transmittedDirection));
        float effectiveThickness = thickness / cosl;
        float3 fullCoverageTransmittedLight = lightColor * exp(-effectiveThickness * transmittance);
        float3 transmittedLight = lerp(lightColor, fullCoverageTransmittedLight, coverage);
        // TODO: Second Fresnel?
        float3 f = fresnel.getFresnelTerm(view, light);
        reflected = specular.evaluateSpecularTerm(normal, view, light, f, roughness * roughness) * lightColor;
        return (1.f - f) * transmittedLight;
    }

    public float3 transmitBackwards(float3 view, out float3 refractedView, out float3 internalReflectionColor)
    {
        refractedView = -refract(-view, normal, ior);
        float cosl = saturate(dot(normal, refractedView));
        float effectiveThickness = thickness / cosl;
        float3 fullCoverageTransmittedLight = exp(-effectiveThickness * transmittance);
        // TODO: Fresnel?
        internalReflectionColor = 0.f;
        return lerp(1.f, fullCoverageTransmittedLight, coverage);
    }

    public float3 evaluateEmissive(float3 view)
    {
        return emissive;
    }
}

public typedef BeerLambertTopLayerBSDF<DefaultFresnel, DefaultSpecularTerm> DefaultTopLayerBSDF;
