
struct Vertex
{
    float3 position;
    float3 color;
    float2 texCoord;
}

struct UniformBufferObject
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
}

[[vk::binding(0, 0)]] ParameterBlock<UniformBufferObject> gUBO;

[[vk::binding(1, 0)]] Sampler2D<float4> texture;

struct VOut
{
    float4 position : SV_Position;
    float2 texCoord;
}

[shader("vertex")]
VOut vertexMain(Vertex input)
{
    VOut output;
    output.texCoord = input.texCoord;
    output.position = mul(gUBO.projection, mul(gUBO.view, mul(gUBO.model, float4(input.position, 1.))));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VOut input) : SV_Target
{
    return texture.Sample(input.texCoord);
}