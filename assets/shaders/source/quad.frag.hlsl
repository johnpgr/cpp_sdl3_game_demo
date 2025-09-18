struct FSInput {
    float4 position : SV_Position;
    float2 texture_coords : TEXCOORD0;
};

Texture2D<float4> texture_atlas : register(t0, space2);
SamplerState texture_sampler : register(s0, space2);

float4 main(FSInput input) {
    // Sample texture using point sampling (equivalent to texelFetch in OpenGL)
    int2 texture_coords = int2(input.texture_coords);
    float4 texture_color = texture_atlas.Sample(texture_sampler, input.texture_coords);
    
    // Discard transparent pixels
    if (texture_color.a == 0.0) {
        discard;
    }
    
    return texture_color;
}
