
//--------------------------------------------------------------------------------------

// File: VoxelShaders.fxh
// Copyright (c) Kyung Hee University.

//--------------------------------------------------------------------------------------

#define NUM_LIGHTS (2)


//--------------------------------------------------------------------------------------

// Global Variables

//--------------------------------------------------------------------------------------

/*--------------------------------------------------------------------

  TODO: Declare texture array and sampler state array for diffuse texture and normal texture (remove the comment)

--------------------------------------------------------------------*/

Texture2D txDiffuse[2] : register(t0);
SamplerState samLinear[2] : register(s0);

//--------------------------------------------------------------------------------------

// Constant Buffer Variables

//--------------------------------------------------------------------------------------

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C

  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation and shading

C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

/*--------------------------------------------------------------------

  TODO: cbChangeOnCameraMovement definition (remove the comment)

--------------------------------------------------------------------*/

cbuffer cbChangeOnCameraMovement : register(b0)
{
    matrix View;
    float4 CameraPosition;
}

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C

  Cbuffer:  cbChangeOnResize

  Summary:  Constant buffer used for projection transformation

C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

/*--------------------------------------------------------------------

  TODO: cbChangeOnResize definition (remove the comment)

--------------------------------------------------------------------*/

cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C

  Cbuffer:  cbChangesEveryFrame

  Summary:  Constant buffer used for world transformation

C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

/*--------------------------------------------------------------------

  TODO: cbChangesEveryFrame definition (remove the comment)

//-------------------------------------------------------------------*/

cbuffer cbChangeEveryFrame : register(b2)
{

    matrix World;
    float4 OutputColor;
    bool HasNormalMap;

};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C

  Cbuffer:  cbLights

  Summary:  Constant buffer used for shading

C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

/*--------------------------------------------------------------------

  TODO: cbLights definition (remove the comment)

--------------------------------------------------------------------*/

cbuffer cbLights : register(b3)
{
    float4 LightPositions[NUM_LIGHTS];
    float4 LightColors[NUM_LIGHTS];
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C

  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader, 

            instance data included

C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

/*--------------------------------------------------------------------

  TODO: VS_INPUT definition (remove the comment)

--------------------------------------------------------------------*/

struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    row_major matrix mTransform : INSTANCE_TRANSFORM;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C

  Struct:   PS_INPUT

  Summary:  Used as the input to the pixel shader, output of the 

            vertex shader

C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

/*--------------------------------------------------------------------

  TODO: PS_INPUT definition (remove the comment)

--------------------------------------------------------------------*/

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float3 WorldPosition : WORLDPOS;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};



//--------------------------------------------------------------------------------------

// Vertex Shader

//--------------------------------------------------------------------------------------


PS_INPUT VSVoxel(VS_INPUT input)
{

    PS_INPUT output = (PS_INPUT) 0;

    output.Position = mul(input.Position, input.mTransform);
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    output.Normal = normalize(mul(float4(input.Normal, 0), World).xyz);

    if (HasNormalMap)
    {
        output.Tangent = normalize(mul(float4(input.Tangent, 0.0f), World).xyz);
        output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
    }

    output.WorldPosition = mul(input.Position, input.mTransform);
    output.WorldPosition = mul(float4(output.WorldPosition, 1), World);
    output.TexCoord = input.TexCoord;

    return output;

}



//--------------------------------------------------------------------------------------

// Pixel Shader

//--------------------------------------------------------------------------------------

float4 PSVoxel(PS_INPUT input) : SV_Target
{
    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float3 viewDirection = normalize(CameraPosition.xyz - input.WorldPosition);
    float3 textureColor = txDiffuse[0].Sample(samLinear[0], input.TexCoord);
    
    float3 normal = normalize(input.Normal);

    if (HasNormalMap)
    {
        // sample the pixel in the normal map
        float4 bumpMap = txDiffuse[1].Sample(samLinear[1], input.TexCoord);
        // expand the range of the normal value from (0,1)to(-1,1)
        bumpMap = (bumpMap * 2.0f) - 1.0f;
        // calculate the normal from the data in the normal map
        float3 bumpNormal = (bumpMap.x * input.Tangent) + (bumpMap.y * input.Bitangent) + (bumpMap.z * normal);
        // normalize the resulting bump normal and replace existing normal
        normal = normalize(bumpNormal);

    }

    for (uint i = 0; i < NUM_LIGHTS; i++)
    {
        ambient += float3(0.1f, 0.1f, 0.1f) * LightColors[i].xyz;
        float3 lightDirection = normalize(LightPositions[i].xyz - input.WorldPosition);
        diffuse += saturate(dot(normal, lightDirection) * LightColors[i].xyz);
        float3 reflectDirection = reflect(-lightDirection, normal);
        specular += pow(saturate(dot(reflectDirection, viewDirection)), 20.0f) * LightColors[i].xyz;
    }
    return float4((ambient + diffuse + specular) * textureColor, 1.0f);

}