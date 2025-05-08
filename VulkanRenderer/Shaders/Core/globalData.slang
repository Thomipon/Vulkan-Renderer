module globalData;

public struct ViewData
{
    public float4x4 viewProjection;
    public float3 viewPosition;
    public float exposureValue;
}

public struct ModelData
{
    public float4x4 modelTransform;
    public float4x4 inverseTransposeModelTransform;
}
