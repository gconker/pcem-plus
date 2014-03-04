//
// Shader that does nothing, returns accurate picture without filtering
//
// Accurate
//

#include "Scaling.inc"

// The name of this effect
string name : NAME = "Accurate";


//Global Variables
sampler2D g_samSrcColor;
float Timer : TIME;


//
// Techniques
//

// combineTechnique: Final combine steps. Outputs to destination frame buffer
string combineTechique : COMBINETECHNIQUE = "Accurate";

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

float4 PS ( float2 Tex : TEXCOORD0  ) : COLOR0
{
	float4 Color;
 	Color = tex2D( g_samSrcColor, Tex.xy);
  	return Color;
}


//
// Technique
//

technique Accurate
{
    pass P0
    {
        // shaders
        VertexShader = compile vs_1_1 VS();
        PixelShader  = compile ps_1_1 PS();
  

    }  
}
