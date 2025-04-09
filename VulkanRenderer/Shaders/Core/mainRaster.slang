module mainRaster;

import lights;
import material;
import geometry;

struct ViewData
{
    float4x4 viewProjection;
    float3 viewPosition;
}

uniform ViewData gViewData;

struct ModelData
{
    float4x4 modelTransform;
    float4x4 inverseTransposeModelTransform;
}

uniform ModelData gModelData;

uniform ILightEnvironment gLightEnvironment;
uniform IMaterial gMaterial;

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 textureCoordinate : UV;
}

struct ProcessedVertex
{
    float3 worldPosition;
    float3 worldNormal;
    float2 textureCoordinate;
}

struct VertexOutput
{
    ProcessedVertex vertex : Vertex;
    float4 sv_position : SV_Position;
}


[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.vertex.worldPosition = mul(gModelData.modelTransform, float4(input.position, 1.)).xyz;
    output.vertex.worldNormal = mul(gModelData.inverseTransposeModelTransform, float4(input.normal, 0.)).xyz;
    output.vertex.textureCoordinate = input.textureCoordinate;
    output.sv_position = mul(gViewData.viewProjection, float4(output.vertex.worldPosition, 1.));
    return output;
}

[shader("fragment")]
float4 fragmentMain(ProcessedVertex vertex : Vertex) : SV_Target
{
    SurfaceGeometry geometry;
    geometry.worldPosition = vertex.worldPosition;
    geometry.worldNormal = vertex.worldNormal;
    geometry.textureCoordinate = vertex.textureCoordinate;

    float3 viewDirection = normalize(gViewData.viewPosition - geometry.worldPosition);

    let materialResult = gMaterial.evaluate(geometry);
    float3 color = saturate(gLightEnvironment.illuminate(materialResult.geometry, materialResult.brdf, viewDirection));
    return float4(color, 1.);
}