module geometry;

import globalData;

public struct SurfaceGeometry
{
    public float3 worldPosition;
    public float3 worldNormal;
    public float2 textureCoordinate;
    public ModelData modelData;

    public float3 getLocalPosition()
    {
        return mul(transpose(modelData.inverseTransposeModelTransform), float4(worldPosition, 1.f)).xyz;
    }
}