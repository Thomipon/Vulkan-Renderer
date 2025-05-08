module mainRaster;

import lights;
import material;
import geometry;
import globalData;

uniform ViewData gViewData;
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
    geometry.worldNormal = normalize(vertex.worldNormal);
    geometry.textureCoordinate = vertex.textureCoordinate;
    geometry.modelData = gModelData;

    float3 viewDirection = normalize(gViewData.viewPosition - geometry.worldPosition);

    let materialResult = gMaterial.evaluate(geometry);
    float3 color = max(gLightEnvironment.illuminate(materialResult.geometry, materialResult.brdf, viewDirection) + materialResult.brdf.evaluateEmissive(viewDirection), 0.f);
    return float4(1.f - exp(-color * gViewData.exposureValue), 1.);
}