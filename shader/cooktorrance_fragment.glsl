#version 330                                                                        
                                                                                    
const int MAX_POINT_LIGHTS = 2;                                                     
                                                                                    
in vec2 TexCoord0;                                                                  
in vec3 Normal0;                                                                    
in vec3 WorldPos0;                                                        
                                                                                    
out vec4 FragColor;                                                                 
                                                                                    
struct BaseLight                                                                    
{                                                                                   
    vec3 Color;                                                                     
    float AmbientIntensity;                                                         
    float DiffuseIntensity;                                                         
};                                                                                  
                                                                                    
struct DirectionalLight                                                             
{                                                                                   
    BaseLight Base;                                                                 
    vec3 Direction;                                                                 
};                                                                                  
                                                                                    
struct Attenuation                                                                  
{                                                                                   
    float Constant;                                                                 
    float Linear;                                                                   
    float Exp;                                                                      
};                                                                                  
                                                                                    
struct PointLight                                                                           
{                                                                                           
    BaseLight Base;                                                                         
    vec3 Position;                                                                          
    Attenuation Atten;                                                                      
};                                                                                          
                                                                                            
uniform int gNumPointLights;                                                               
uniform DirectionalLight gDirectionalLight;                                                 
uniform PointLight gPointLights[MAX_POINT_LIGHTS];                                          
uniform sampler2D gSampler;                                                                 
uniform vec3 gEyeWorldPos;
uniform float gRoughnessvalue; 
uniform float gFresnelF0;
uniform float gK;                                                           
                                                                                            
vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal)                   
{  

// light = ambient + Light.Color * (N * Li) * (k * diffuse +(1-k) * specular);
// specular = FDG/pi * (N * V) * (N * L)
                                                                                        
    vec4 AmbientColor = vec4(Light.Color * Light.AmbientIntensity, 1.0f);

    float DiffuseFactor = dot(Normal, -LightDirection);                                     
                                                                                            
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                                  
    vec4 SpecularColor = vec4(0, 0, 0, 0); 
	                                          
                                                                                            
    if (DiffuseFactor > 0) {                                                                
        DiffuseColor = vec4(Light.Color * Light.DiffuseIntensity * DiffuseFactor, 1.0f);    
                                                                                            
        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);                             
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));  
	              
 //specular light
 //http://ruh.li/GraphicsCookTorrance.html

        vec3 halfVector = normalize(LightDirection + VertexToEye); 
		float NdotH = max(dot(Normal, -halfVector), 0.0);
		float NdotV = max(dot(Normal, -VertexToEye), 0.0);
		float VdotH = max(dot(VertexToEye, halfVector), 0.0);
		float HdotL = max(dot(halfVector, LightDirection), 0.0);

// geometry attenuation
		float gm = 2 * (NdotH * NdotV) / HdotL;
		float gs = 2 * (NdotH * DiffuseFactor) / VdotH;  
		float g = min(1.0, min(gm,gs));
		g = max(0 , g);
		
//microfacet distribution function
//Beckmann distribution function
        float r1 = 1.0 / (4.0 * gRoughnessvalue * gRoughnessvalue * pow(NdotH, 4.0));
		//float r2 = (NdotH * NdotH - 1.0) / (gRoughnessvalue * gRoughnessvalue * NdotH * NdotH);
		float r2 = (NdotH * NdotH) / (gRoughnessvalue * gRoughnessvalue) / (NdotH * NdotH - 1.0);
		float roughness = r1 * exp(r2);   
		
		//fresnel Schlick approximation
        float fresnel = pow(1.0 - VdotH, 5.0);
	    fresnel *= (1.0 - gFresnelF0);
	    fresnel += gFresnelF0;
		float specular = (fresnel * g * roughness) / (NdotV * DiffuseFactor * 3.1415926);
		specular = clamp(specular, 0.0f, 1.0f);
		SpecularColor = vec4(Light.Color * DiffuseFactor * specular, 1.0);  
	}
                                                                                  
    return (AmbientColor + gK * DiffuseColor  + (1-gK)* SpecularColor);                                   
}   

                                                                                        
                                                                                            
vec4 CalcDirectionalLight(vec3 Normal)                                                      
{                                                                                           
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal); 
}                                                                                           
                                                                                            
vec4 CalcPointLight(int Index, vec3 Normal)                                                 
{                                                                                           
    vec3 LightDirection = WorldPos0 - gPointLights[Index].Position;                         
    float Distance = length(LightDirection);                                                
    LightDirection = normalize(LightDirection);                                             
                                                                                            
    vec4 Color = CalcLightInternal(gPointLights[Index].Base, LightDirection, Normal);       
    float Attenuation =  gPointLights[Index].Atten.Constant +                               
                         gPointLights[Index].Atten.Linear * Distance +                      
                         gPointLights[Index].Atten.Exp * Distance * Distance;               
                                                                                            
    return Color / Attenuation;                                                             
}                                                                                           
                                                                                            
void main()                                                                                 
{                                                                                           
    vec3 Normal = normalize(Normal0);                                                       
    vec4 TotalLight = CalcDirectionalLight(Normal);                                  
                                                                                            
    for (int i = 0 ; i < gNumPointLights ; i++) {                                           
        TotalLight += CalcPointLight(i, Normal);                                            
    }                                                                                       
	                                                                                      
    FragColor = texture2D(gSampler, TexCoord0.xy) * TotalLight;                             
}
