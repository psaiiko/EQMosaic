cbuffer PerFrame : register(b0)
{
    float g_showBorder;
    float g_aspectRatioOut;
    float g_aspectRatioIn;
    float g_cropToFill; // 0 = contain, 1 = cover
    float4 g_borderColor; 
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(PS_IN input) : SV_Target
{
    float2 uv = input.uv;
    float2 scale = float2(1.0, 1.0);
    bool wider = g_aspectRatioOut > g_aspectRatioIn;

    if (g_cropToFill > 0.5)
    {
        // COVER (some areas no longer visible)
        
        if (wider)
            // crop top/bottom
            scale.y = g_aspectRatioIn / g_aspectRatioOut;
        else
            // crop left/right
            scale.x = g_aspectRatioOut / g_aspectRatioIn;
    }
    else
    {
        // CONTAIN (letterbox / pillarbox)
        if (wider)
            // pillarbox left/right
            scale.x = g_aspectRatioOut / g_aspectRatioIn;
        else
            // letterbox top/bottom
            scale.y = g_aspectRatioIn / g_aspectRatioOut;
    }

    uv = (uv - 0.5) * scale + 0.5;

    bool outside = uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;

    float4 color;
    if (outside && g_cropToFill < 0.5)
        // contain mode bars
        color = float4(0.0, 0.0, 0.0, 1.0);
    else
        color = float4(g_texture.Sample(g_sampler, uv).rgb,0.5);

    // Viewport border highlight
    float2 margin = 2.0 * float2(0.01 / g_aspectRatioOut, 0.01);

    float border = max(smoothstep(1.0 - margin.x,1.0,input.uv.x),smoothstep(margin.x,0.0,input.uv.x));
    border = max(border,smoothstep(1.0 - margin.y,1.0,input.uv.y));
    border = max(border,smoothstep(margin.y,0.0,input.uv.y));
    border *= g_showBorder;

    // desaturation / tint for inactive players
    float luminance = dot(color.rgb,float3(0.299, 0.587, 0.114));

    color.rgb = lerp(float3(luminance, luminance, luminance) * float3(0.8, 0.8, 1.0), color.rgb, 0.1 + 0.9 * g_showBorder);

    // final
    return lerp(color, g_borderColor, border);
}
