struct Transform {
    int2 atlas_offset;
    int2 sprite_size;
    float2 pos;
    float2 size;
};

StructuredBuffer<Transform> transforms : register(t0);

cbuffer Constants : register(b0) {
    float2 screen_size;
    float4x4 camera_matrix;
}

struct VSInput {
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct VSOutput {
    float4 position : SV_Position;
    float2 texture_coords : TEXCOORD0;
};

VSOutput main(VSInput input) {
    VSOutput output;
    
    Transform transform = transforms[input.instanceID];

    float2 vertices[4] = {
        transform.pos,                                          // TL
        float2(transform.pos + float2(0.0, transform.size.y)),  // BL
        float2(transform.pos + float2(transform.size.x, 0.0)),  // TR
        transform.pos + transform.size                          // BR
    };

    int indices[6] = { 0, 1, 2, 2, 1, 3 };

    float left   = transform.atlas_offset.x;
    float top    = transform.atlas_offset.y;
    float right  = transform.atlas_offset.x + transform.sprite_size.x;
    float bottom = transform.atlas_offset.y + transform.sprite_size.y;

    float2 texture_coords[4] = {
        float2(left, top),
        float2(left, bottom),
        float2(right, top),
        float2(right, bottom)
    };

    float2 vertex_pos = vertices[indices[input.vertexID]];
    // vertex_pos.y = -vertex_pos.y + screen_size.y;
    // vertex_pos = 2.0 * (vertex_pos / screen_size) - 1.0;
    output.position = mul(camera_matrix, float4(vertex_pos, 0.0, 1.0));

    output.texture_coords = texture_coords[indices[input.vertexID]];
    
    return output;
}
