module lights;

import brdf;
import geometry;
import indirectLighting;

public interface ILightEnvironment
{
    float3 illuminate<B:IBRDF>(SurfaceGeometry geometry, B brdf, float3 viewDirection);
}

struct LightArray<L : ILightEnvironment, let N : int> : ILightEnvironment
{
    int count;
    L lights[N];

    float3 illuminate<B:IBRDF>(SurfaceGeometry geometry, B brdf, float3 viewDirection)
    {
        float3 sum = 0.;
        for (int i = 0; i < count; ++i)
        {
            sum += lights[i].illuminate(geometry, brdf, viewDirection);
        }
        return sum;
    }
}

struct LightPair<T : ILightEnvironment, U : ILightEnvironment> : ILightEnvironment
{
    T first;
    U second;

    float3 illuminate<B:IBRDF>(SurfaceGeometry geometry, B brdf, float3 viewDirection)
    {
        return first.illuminate(geometry, brdf, viewDirection) + second.illuminate(geometry, brdf, viewDirection);
    }
};

struct EmptyLight : ILightEnvironment
{
    float3 illuminate<B:IBRDF>(SurfaceGeometry geometry, B brdf, float3 viewDirection)
    {
        return 0.;
    }
}

struct DirectionalLight : ILightEnvironment
{
    float3 direction;
    float3 color;
    float intensity;

    float3 illuminate<B:IBRDF>(SurfaceGeometry geometry, B brdf, float3 viewDirection)
    {
        float3 realColor = intensity * color;
        return brdf.evaluate(viewDirection, direction, realColor);
    }
}

struct PointLight : ILightEnvironment
{
    float3 position;
    float3 color;
    float intensity;

    float3 illuminate<B:IBRDF>(SurfaceGeometry geometry, B brdf, float3 viewDirection)
    {
        float3 realColor = intensity * color;
        float3 delta = geometry.worldPosition - position;
        float distance = length(delta);
        float3 direction = normalize(delta);
        float3 illumination = realColor / (distance * distance);
        return brdf.evaluate(viewDirection, direction, illumination);
    }
}

struct AmbientLight : ILightEnvironment, IIndirectLightEnvironment
{
    float3 color;
    float intensity;

    float3 illuminate<B:IBRDF>(SurfaceGeometry geometry, B brdf, float3 viewDirection)
    {
        return brdf.evaluateIndirect(viewDirection, color * intensity, this);
    }

    float3 sampleEnvironment(float3 direction, float3 footPoint)
    {
        return color * intensity;
    }
}
