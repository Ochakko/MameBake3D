//--------------------------------------------------------------------------------------
// 
// Copyright (c) ‚¨‚¿‚á‚Á‚±LAB. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
texture g_MeshTexture;              // Color texture for mesh

float4 g_diffuse;
float3 g_ambient;
float3 g_specular;
float  g_power;
float3 g_emissive;

float3 g_LightDir[3];               // Light's direction in world space
float4 g_LightDiffuse[3];           // Light's diffuse color

float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mVP;    // View * Projection matrix
float3	 g_EyePos;

float4 g_m3x4Mat[70][3];

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
sampler MeshTextureSampler = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};



//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
	float3 Specular	  : TEXCOORD0;
    float2 TextureUV  : TEXCOORD1;  // vertex texture coords 
};

struct VS_LINEOUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
};

struct VS_SPRITEOUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  : TEXCOORD1;  // vertex texture coords 
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneBoneVS( float4 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float2 vTexCoord0 : TEXCOORD0,
						 float4 bweight : BLENDWEIGHT,
						 float4 bindices : BLENDINDICES,
                         uniform int nNumLights )
{
    VS_OUTPUT Output;
    float4 wPos;

	int bi[4] = { bindices.x, bindices.y, bindices.z, bindices.w };
	float bw[4] = { bweight.x, bweight.y, bweight.z, bweight.w };

	float4x4 finalmat = 0;
    for(int i=0; i<4; i++ ){
		float4x4 addmat;
		addmat = float4x4(
			g_m3x4Mat[bi[i]][0].x, g_m3x4Mat[bi[i]][0].y, g_m3x4Mat[bi[i]][0].z, 0.0f,
			g_m3x4Mat[bi[i]][0].w, g_m3x4Mat[bi[i]][1].x, g_m3x4Mat[bi[i]][1].y, 0.0f,
			g_m3x4Mat[bi[i]][1].z, g_m3x4Mat[bi[i]][1].w, g_m3x4Mat[bi[i]][2].x, 0.0f,
			g_m3x4Mat[bi[i]][2].y, g_m3x4Mat[bi[i]][2].z, g_m3x4Mat[bi[i]][2].w, 1.0f );
		finalmat += bw[i] * addmat;
	}	
	
	wPos = mul( vPos, finalmat );
	Output.Position = mul( wPos, g_mVP );
	wPos /= wPos.w;
    
    float3 wNormal;
    wNormal = normalize(mul(vNormal, (float3x3)finalmat)); // normal (world space)
    
    float3 totaldiffuse = float3(0,0,0);
    float3 totalspecular = float3(0,0,0);
	float calcpower = g_power * 0.1f;

    for(int i=0; i<nNumLights; i++ ){
		float nl;
		float3 h;
		float nh;	
		float4 tmplight;
		
		nl = dot( wNormal, g_LightDir[i] );
		h = normalize( ( g_LightDir[i] + g_EyePos - wPos.xyz ) * 0.5f );
		nh = dot( wNormal, h );

        totaldiffuse += g_LightDiffuse[i] * max(0,dot(wNormal, g_LightDir[i]));
		totalspecular +=  ((nl) < 0) || ((nh) < 0) ? 0 : ((nh) * calcpower);
	}

    Output.Diffuse.rgb = g_diffuse.rgb * totaldiffuse.rgb 
		+ g_ambient + g_emissive;   
    Output.Diffuse.a = g_diffuse.a; 
    
	Output.Specular = g_specular * totalspecular;

    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}
VS_OUTPUT RenderSceneBoneNLightVS( float4 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float2 vTexCoord0 : TEXCOORD0,
						 float4 bweight : BLENDWEIGHT,
						 float4 bindices : BLENDINDICES )
{
    VS_OUTPUT Output;
    float4 wPos;

	int bi[4] = { bindices.x, bindices.y, bindices.z, bindices.w };
	float bw[4] = { bweight.x, bweight.y, bweight.z, bweight.w };

	float4x4 finalmat = 0;
    for(int i=0; i<4; i++ ){
		float4x4 addmat;
		addmat = float4x4(
			g_m3x4Mat[bi[i]][0].x, g_m3x4Mat[bi[i]][0].y, g_m3x4Mat[bi[i]][0].z, 0.0f,
			g_m3x4Mat[bi[i]][0].w, g_m3x4Mat[bi[i]][1].x, g_m3x4Mat[bi[i]][1].y, 0.0f,
			g_m3x4Mat[bi[i]][1].z, g_m3x4Mat[bi[i]][1].w, g_m3x4Mat[bi[i]][2].x, 0.0f,
			g_m3x4Mat[bi[i]][2].y, g_m3x4Mat[bi[i]][2].z, g_m3x4Mat[bi[i]][2].w, 1.0f );
		finalmat += bw[i] * addmat;
	}	

	wPos = mul( vPos, finalmat );
	Output.Position = mul( wPos, g_mVP );
	wPos /= wPos.w;
    
    float3 wNormal;
    wNormal = normalize(mul(vNormal, (float3x3)finalmat)); // normal (world space)
    
    Output.Diffuse.rgb = g_diffuse.rgb + g_ambient + g_emissive;   
    Output.Diffuse.a = g_diffuse.a; 
    
	Output.Specular = float3( 0.0f, 0.0f, 0.0f );

    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}

VS_OUTPUT RenderSceneNoBoneVS( float4 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float2 vTexCoord0 : TEXCOORD0,
                         uniform int nNumLights )
{
    VS_OUTPUT Output;
    float4 wPos;

	float4x4 finalmat = g_mWorld;

	wPos = mul( vPos, finalmat );
	Output.Position = mul( wPos, g_mVP );
	wPos /= wPos.w;
    
    float3 wNormal;
    wNormal = normalize(mul(vNormal, (float3x3)finalmat)); // normal (world space)
    
    float3 totaldiffuse = float3(0,0,0);
    float3 totalspecular = float3(0,0,0);
	float calcpower = g_power * 0.1f;

    for(int i=0; i<nNumLights; i++ ){
		float nl;
		float3 h;
		float nh;	
		float4 tmplight;
		
		nl = dot( wNormal, g_LightDir[i] );
		h = normalize( ( g_LightDir[i] + g_EyePos - wPos.xyz ) * 0.5f );
		nh = dot( wNormal, h );

        totaldiffuse += g_LightDiffuse[i] * max(0,dot(wNormal, g_LightDir[i]));
		totalspecular +=  ((nl) < 0) || ((nh) < 0) ? 0 : ((nh) * calcpower);
	}

    Output.Diffuse.rgb = g_diffuse.rgb * totaldiffuse.rgb 
		+ g_ambient + g_emissive;   
    Output.Diffuse.a = g_diffuse.a; 
    
	Output.Specular = g_specular * totalspecular;

    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}

VS_OUTPUT RenderSceneNoBoneNLightVS( float4 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float2 vTexCoord0 : TEXCOORD0 )
{
    VS_OUTPUT Output;
    float4 wPos;

	float4x4 finalmat = g_mWorld;

	wPos = mul( vPos, finalmat );
	Output.Position = mul( wPos, g_mVP );
	wPos /= wPos.w;
    
    float3 wNormal;
    wNormal = normalize(mul(vNormal, (float3x3)finalmat)); // normal (world space)
    

    Output.Diffuse.rgb = g_diffuse.rgb + g_ambient + g_emissive;   
    Output.Diffuse.a = g_diffuse.a; 
    
	Output.Specular = float3( 0.0f, 0.0f, 0.0f );

    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}


VS_LINEOUTPUT RenderSceneLineVS( float4 vPos : POSITION )
{
    VS_LINEOUTPUT Output;
    float4 wPos;

	float4x4 finalmat = g_mWorld;

	wPos = mul( vPos, finalmat );
	Output.Position = mul( wPos, g_mVP );


    Output.Diffuse.rgb = g_diffuse.rgb;
	Output.Diffuse.a = g_diffuse.a; 
        
    return Output;    
}


VS_SPRITEOUTPUT RenderSceneSpriteVS( float4 vPos : POSITION, 
                         float2 vTexCoord0 : TEXCOORD0 )
{
    VS_SPRITEOUTPUT Output;
	Output.Position = vPos;
    Output.Diffuse.rgb = g_diffuse.rgb;   
    Output.Diffuse.a = g_diffuse.a; 
    Output.TextureUV = vTexCoord0; 
    
    return Output;    
}



//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderScenePSTex( VS_OUTPUT In ) 
{ 
    PS_OUTPUT Output;
    Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV) * In.Diffuse + float4( In.Specular, 0 );
    return Output;
}

PS_OUTPUT RenderScenePSNotex( VS_OUTPUT In ) 
{ 
    PS_OUTPUT Output;
    Output.RGBColor = In.Diffuse + float4( In.Specular, 0 );
    return Output;
}

PS_OUTPUT RenderScenePSLine( VS_LINEOUTPUT In ) 
{ 
    PS_OUTPUT Output;
    Output.RGBColor = In.Diffuse;
    return Output;
}

PS_OUTPUT RenderScenePSSprite( VS_SPRITEOUTPUT In ) 
{ 
    PS_OUTPUT Output;
    Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV) * In.Diffuse;
    return Output;
}


//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------
technique RenderBoneL0
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneBoneNLightVS();
        PixelShader  = compile ps_3_0 RenderScenePSTex();
    }
    pass P1
    {          
        VertexShader = compile vs_3_0 RenderSceneBoneNLightVS();
        PixelShader  = compile ps_3_0 RenderScenePSNotex();
    }

}

technique RenderBoneL1
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneBoneVS( 1 );
        PixelShader  = compile ps_3_0 RenderScenePSTex();
    }
    pass P1
    {          
        VertexShader = compile vs_3_0 RenderSceneBoneVS( 1 );
        PixelShader  = compile ps_3_0 RenderScenePSNotex();
    }

}

technique RenderBoneL2
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneBoneVS( 2 );
        PixelShader  = compile ps_3_0 RenderScenePSTex();
    }
    pass P1
    {          
        VertexShader = compile vs_3_0 RenderSceneBoneVS( 2 );
        PixelShader  = compile ps_3_0 RenderScenePSNotex();
    }

}

technique RenderBoneL3
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneBoneVS( 3 );
        PixelShader  = compile ps_3_0 RenderScenePSTex();
    }
    pass P1
    {          
        VertexShader = compile vs_3_0 RenderSceneBoneVS( 3 );
        PixelShader  = compile ps_3_0 RenderScenePSNotex();
    }

}

technique RenderNoBoneL0
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneNoBoneNLightVS();
        PixelShader  = compile ps_3_0 RenderScenePSTex();
    }
    pass P1
    {          
        VertexShader = compile vs_3_0 RenderSceneNoBoneNLightVS();
        PixelShader  = compile ps_3_0 RenderScenePSNotex();
    }

}


technique RenderNoBoneL1
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneNoBoneVS( 1 );
        PixelShader  = compile ps_3_0 RenderScenePSTex();
    }
    pass P1
    {          
        VertexShader = compile vs_3_0 RenderSceneNoBoneVS( 1 );
        PixelShader  = compile ps_3_0 RenderScenePSNotex();
    }

}

technique RenderNoBoneL2
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneNoBoneVS( 2 );
        PixelShader  = compile ps_3_0 RenderScenePSTex();
    }
    pass P1
    {          
        VertexShader = compile vs_3_0 RenderSceneNoBoneVS( 2 );
        PixelShader  = compile ps_3_0 RenderScenePSNotex();
    }

}

technique RenderNoBoneL3
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneNoBoneVS( 3 );
        PixelShader  = compile ps_3_0 RenderScenePSTex();
    }
    pass P1
    {          
        VertexShader = compile vs_3_0 RenderSceneNoBoneVS( 3 );
        PixelShader  = compile ps_3_0 RenderScenePSNotex();
    }

}

technique RenderLine
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneLineVS();
        PixelShader  = compile ps_3_0 RenderScenePSLine();
    }
}

technique RenderSprite
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneSpriteVS();
        PixelShader  = compile ps_3_0 RenderScenePSSprite();
    }
}
