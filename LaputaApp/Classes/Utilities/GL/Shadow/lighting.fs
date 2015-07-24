#version 120
                                                                                    
in vec4 LightSpacePos;                                                              
in vec2 TexCoord0;                                                                  
in vec3 Normal0;                                                                    
in vec3 WorldPos0;
                                                                                    
struct BaseLight                                                                    
{                                                                                   
    vec3 Color;
};                                                                                  
                                                                                     
struct PointLight                                                                           
{                                                                                           
    BaseLight Base;                                                                  
    vec3 Position;
};

uniform PointLight gPointLight;
uniform sampler2D gSampler;                                                                 
uniform sampler2D gShadowMap;
                                                                                            
float CalcShadowFactor(vec4 LightSpacePos)                                                  
{                                                                                           
    vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w;                                  
    vec2 UVCoords;                                                                          
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;                                                  
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;                                                  
    float z = 0.5 * ProjCoords.z + 0.5;                                                     
    float Depth = texture(gShadowMap, UVCoords).x;                                          
    if (Depth < z + 0.00001)                                                                 
        return 0.5;                                                                         
    else                                                                                    
        return 1.0;                                                                         
}

vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal,            
                       float ShadowFactor)                                                  
{
    float DiffuseFactor = dot(Normal, -LightDirection);                                     
                                                                                            
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);
                                                                                            
    if (DiffuseFactor > 0) {                                                                
        DiffuseColor = vec4(Light.Color, 1.0f) * DiffuseFactor;
    }                                                                                       
                                                                                            
    return ShadowFactor * DiffuseColor;
}
                                                                                            
vec4 CalcPointLight(PointLight l, vec3 Normal, vec4 LightSpacePos)                   
{                                                                                           
    vec3 LightDirection = WorldPos0 - l.Position;                                             
    LightDirection = normalize(LightDirection);                                             
    float ShadowFactor = CalcShadowFactor(LightSpacePos);                                   
                                                                                            
    vec4 Color = CalcLightInternal(l.Base, LightDirection, Normal, ShadowFactor);           
    
    return Color;
}

void main()                                                                                 
{                                                                                           
    vec3 Normal = normalize(Normal0);                                                       
    vec4 TotalLight = CalcPointLight(gPointLight, Normal, LightSpacePos);
    
    //texture color only
    vec4 SampledColor = texture2D(gSampler, TexCoord0.xy);                                  
    gl_FragColor = SampledColor * TotalLight;
}
