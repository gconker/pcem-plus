//
// 2D Scale Effect File
//
// Cartoon
//

#include "Scaling.inc"

// The name of this effect
string name : NAME = "Cartoon";


//Global Variables
sampler2D s0;
float Timer : Time;


//
// Techniques
//

// combineTechnique: Final combine steps. Outputs to destination frame buffer
string combineTechique : COMBINETECHNIQUE = "Cartoon";

// Input and Output Semantics 
struct VS_INPUT 
{ 
   float4 position : POSITION; 
   float2 texCoord : TEXCOORD; 
}; 
struct VS_OUTPUT 
{ 
   float4 position : POSITION; 
   float2 texCoord : TEXCOORD;    
}; 
#define PS_INPUT VS_OUTPUT 



//
// Vertex Shader
//

VS_OUTPUT VS( const VS_INPUT IN)
{
	 VS_OUTPUT OUT; 
   OUT.position = mul(IN.position, WorldViewProjection); 
   OUT.texCoord = IN.texCoord; 
   return OUT; 
}


//
// Pixel Shader
//

float4 PS ( float2 tex : TEXCOORD0  ) : COLOR0
{
   const float scale_factor = 4.0;
   float texsizeX = 2048.0f;
   float texsizeY = 2048.0f;
   float dx = 0.5 / (texsizeX * scale_factor);
   float dy = 0.5 / (texsizeY * scale_factor);


   float3 c00 = tex2D(s0, tex + float2(-dx, -dy)).xyz;
   float3 c01 = tex2D(s0, tex + float2(-dx, 0)).xyz;
   float3 c02 = tex2D(s0, tex + float2(-dx, dy)).xyz;
   float3 c10 = tex2D(s0, tex + float2(0, -dy)).xyz;
   float3 c11 = tex2D(s0, tex + float2(0, 0)).xyz;
   float3 c12 = tex2D(s0, tex + float2(0, dy)).xyz;
   float3 c20 = tex2D(s0, tex + float2(dx, -dy)).xyz;
   float3 c21 = tex2D(s0, tex + float2(dx, 0)).xyz;
   float3 c22 = tex2D(s0, tex + float2(dx, dy)).xyz;


   
   float3 first = lerp(c00, c20, frac(scale_factor * tex.x * texsizeX + 0.5));
   float3 second = lerp(c02, c22, frac(scale_factor * tex.x * texsizeY + 0.5));


   float3 mid_horiz = lerp(c01, c21, frac(scale_factor * tex.x * texsizeX + 0.5));
   float3 mid_vert = lerp(c10, c12, frac(scale_factor * tex.y * texsizeY + 0.5));


   float3 res = lerp(first, second, frac(scale_factor * tex.y * texsizeY + 0.5));
   return float4(0.28 * (res + mid_horiz + mid_vert) + 4.7 * abs(res - lerp(mid_horiz, mid_vert, 0.5)), 1.0);
}


//
// Technique
//

technique Cartoon
{
    pass P0
    {
        // shaders
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PS();
  

    }  
}
