module topLayerBSDF;

import Core.indirectLighting;

import Library.fresnel;
import Library.specular;

public interface ITopLayerBSDF
{
    float3 transmitForward(float3 light, float3 view, float3 lightColor, out float3 reflected, out float3 transmittedDirection, out float3 surfacePenetratingLight);
    float3 transmitBackwards(float3 view, out float3 refractedView, out float3 internalReflectionColor, out float3 absorptionFactor);
    float3 transmitInternal(float3 direction, float roughness, out float3 refractedDirection, out float combinedRoughness);
    float3 evaluateIndirect<Environment : IIndirectLightEnvironment>(float3 viewDirection, Environment environment);
    float3 evaluateEmissive(float3 view);

    // TODO: This may introduce dynamic dispatch if used improperly. It is however the only solution since slang does not support higher kinded types.
    IIndirectLightEnvironment modifyIndirectLightEnvironment<BaseEnvironment : IIndirectLightEnvironment>(BaseEnvironment baseEnvironment);
}

public struct IndirectLightEnvironmentThroughLayer<BaseEnvironment : IIndirectLightEnvironment, Layer : ITopLayerBSDF> : IIndirectLightEnvironment
{
    BaseEnvironment baseEnvironment;
    Layer layer;

    public float3 sampleEnvironment(float3 direction, float roughness)
    {
        float3 refracted;
        float combinedRoughness;
        float3 transmittedScale = layer.transmitInternal(direction, roughness, refracted, combinedRoughness);
        return baseEnvironment.sampleEnvironment(refracted, combinedRoughness) * transmittedScale;
    }
}

public struct BeerLambertTopLayerBSDF<Fresnel : IFresnel, SpecularTerm : ISpecularTerm> : ITopLayerBSDF
{
    public float3 normal;
    public float coverage;
    public float thickness;
    public float roughness;
    public float3 absorption;
    public float ior;
    public Fresnel fresnel;
    public SpecularTerm specular;
    public float3 emissive;
    public float3 ambientOcclusion;

    public float3 transmitForward(float3 light, float3 view, float3 lightColor, out float3 reflected, out float3 transmittedDirection, out float3 surfacePenetratingLight)
    {
        transmittedDirection = -refract(-light, normal, rcp(ior));
        float cosl = saturate(dot(normal, transmittedDirection));
        float effectiveThickness = thickness / cosl;
        float3 fl = fresnel.getFresnelTermDirectly(normal, light);
        surfacePenetratingLight = lightColor * (1.f - fl);
        float3 fullCoverageTransmittedLight = surfacePenetratingLight * exp(-effectiveThickness * absorption);
        float3 transmittedLight = lerp(surfacePenetratingLight, fullCoverageTransmittedLight, coverage);
        float3 f = fresnel.getFresnelTerm(view, light);
        reflected = specular.evaluateSpecularTerm(normal, view, light, f, roughness * roughness) * lightColor * coverage * fl;
        return (1.f - f) * transmittedLight * saturate(dot(light, normal));
    }

    public float3 transmitBackwards(float3 view, out float3 refractedView, out float3 internalReflectionColor, out float3 absorptionFactor)
    {
        refractedView = -refract(-view, normal, rcp(ior));
        float cosl = saturate(dot(normal, refractedView));
        float effectiveThickness = thickness / cosl;
        float3 fullCoverageTransmittedLight = exp(-effectiveThickness * absorption);
        float3 fl = fresnel.getFresnelTermDirectly(normal, view);
        absorptionFactor = lerp(1.f, fullCoverageTransmittedLight, coverage);
        internalReflectionColor = absorptionFactor * fl;
        return absorptionFactor - internalReflectionColor;
    }

    public float3 transmitInternal(float3 direction, float roughness, out float3 refractedDirection, out float combinedRoughness)
    {
        float cosl = saturate(dot(normal, direction));
        float effectiveThickness = thickness / cosl;
        float3 fullCoverageTransmittedLight = exp(-effectiveThickness * absorption);
        float3 transmittedLight = lerp(1.f, fullCoverageTransmittedLight, coverage);
        // TODO: Fresnel?
        refractedDirection = -refract(-direction, normal, ior);
        combinedRoughness = 1.f - ((1.f - this.roughness) * (1.f - roughness));
        return transmittedLight;
    }

    public float3 evaluateIndirect<Environment : IIndirectLightEnvironment>(float3 viewDirection, Environment environment)
    {
        float3 f = fresnel.getFresnelTerm(viewDirection, normal);
        float alpha = roughness * roughness;
        float3 r = -reflect(viewDirection, normal);
        // TODO: Should the occlusion really be here?
        float3 s = specular.evaluateOcclusion(normal, viewDirection, r, alpha) * environment.sampleEnvironment(r, roughness) * f;
        return s * ambientOcclusion * coverage;
    }

    public float3 evaluateEmissive(float3 view)
    {
        return emissive * coverage;
    }

    public IIndirectLightEnvironment modifyIndirectLightEnvironment<BaseEnvironment : IIndirectLightEnvironment>(BaseEnvironment baseEnvironment)
    {
        IndirectLightEnvironmentThroughLayer<BaseEnvironment, This> modified = {};
        modified.baseEnvironment = baseEnvironment;
        modified.layer = this;
        return modified;
    }
}

public typedef BeerLambertTopLayerBSDF<DefaultFresnel, DefaultSpecularTerm> DefaultTopLayerBSDF;
