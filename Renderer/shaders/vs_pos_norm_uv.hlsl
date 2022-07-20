#include "mvp.hlsli" 
 
struct VSIn
{
    float3 posW : POSITION;
    float3 nrm : NORMAL;
    float2 uv : TEXCOORD;
};
 
struct VSOut
{
    float4 posH : SV_Position;
    float4 nrm : NORMAL;
    float4 color : COLOR;
};
 
VSOut main(VSIn input)
{
    VSOut output;
     
    output.posH = mul(mul(mul(float4(input.posW, 1), modeling), view), projection);
    output.nrm = mul(float4(input.nrm, 1), modeling);
    output.color = float4((output.nrm.xyz + 1.0f) * .5f, 1.0f) * .75f;
     
    return output;
 
}