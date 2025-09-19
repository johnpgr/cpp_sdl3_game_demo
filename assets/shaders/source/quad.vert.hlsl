struct SpriteVertex {
    float2 pos;
    float2 size;
    float2 uv_min;    // Normalized UV coordinates
    float2 uv_max;    // Normalized UV coordinates
};

struct VSInput {
    float2 position : POSITION;    // Unit quad vertex (0-1 range)
    float2 uv : TEXCOORD0;         // Unit quad UV (0-1 range)
};

struct VSOutput {
    float4 position : SV_Position;
    float2 texture_coords : TEXCOORD0;
};

StructuredBuffer<SpriteVertex> sprite_vertices : register(t0, space0);

cbuffer Constants : register(b0, space1) {
    float4x4 camera_matrix;
}

VSOutput main(VSInput input, uint instance_id : SV_InstanceID) {
    VSOutput output;
    
    SpriteVertex vertex = sprite_vertices[instance_id];

    // Scale and translate unit quad to world position
    float2 world_pos = vertex.pos + input.position * vertex.size;
    output.position = mul(camera_matrix, float4(world_pos, 0.0f, 1.0f));

    // Interpolate between min and max UV coordinates
    output.texture_coords = lerp(vertex.uv_min, vertex.uv_max, input.uv);
    
    return output;
}
