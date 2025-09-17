Texture2D texture_atlas : register(t0);
SamplerState texture_sampler : register(s0);

struct PSInput {
    float4 position : SV_Position;
    float2 texture_coords : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_Target0;
};

PSOutput main(PSInput input) {
    PSOutput output;
    
    float4 texture_color = texture_atlas.Load(int3(int2(input.texture_coords), 0));

    if (texture_color.a == 0.0) {
        discard;
    }

    output.color = texture_color;
    
    return output;
}
