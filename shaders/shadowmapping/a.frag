#version 460

// layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 lightMVP;
    vec3 lightPos;
    vec3 lightColor;
    vec3 eyePos;
    vec3 ks;
    vec3 kd;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;
layout (binding = 2) uniform sampler2D shadowMap;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location=2) in vec2 fragTexCoord;
layout (location = 3) flat in int modelId_flat;
layout(location =4 ) in vec4 inShadowCoord;
layout(location=5) in vec3 fragViewVec_normalized;
layout(location=6) in vec3 fragLightVec;


layout(location = 0) out vec4 outColor;

vec3 gethalfvec(vec3 a, vec3 b)
{
    return normalize(a+b);
}

vec3 getTexture(sampler2D sampl, vec2 tex)
{
    if(tex.x<-0.5) return ubo.kd;
    return texture(sampl,fragTexCoord).xyz;
}


float shadowFactor(vec4 shadowCoord)
{
    float shadow=1.0;
    if(shadowCoord.z>-1.0 && shadowCoord.z<1.0)
    {
        float dist=texture(shadowMap, shadowCoord.st).r;
        if(shadowCoord.w>0.0 && dist<shadowCoord.z)
        {
            shadow=0.1;
        }
    }
    return shadow;
}

void main() 
{
    // outColor = texture(texSampler, fragTexCoord);

    float shadow=shadowFactor(inShadowCoord/inShadowCoord.w);
    
    float d2reci=1.0/dot(fragLightVec,fragLightVec);

    vec3 lightVec_normalized=normalize(fragLightVec);
    // vec3 eyeVec_normalized=normalize(ubo.eyePos-inPosition);
    vec3 halfVec=gethalfvec(lightVec_normalized,fragViewVec_normalized);
    
    
    float cos0=clamp(dot(halfVec,inNormal),0,1);
    float cos1=clamp(dot(inNormal,lightVec_normalized),0,1);

    vec3 kd_texture=ubo.kd*getTexture(texSampler,fragTexCoord);

    vec3 diffuse=kd_texture*ubo.lightColor*cos1*d2reci;
    vec3 specular=ubo.ks*ubo.lightColor*pow(cos0,30)*d2reci;
    vec3 ambient=vec3(0.02,0.02,0.02)*kd_texture;
    
    if(modelId_flat==0) outColor=0.4*vec4(1,1,1,1);
    else outColor=vec4((specular+diffuse+ambient)*shadow,1.0);

    
    // outColor=clamp(pow(cos0,ubo.specFactor)*vec4(ubo.lightColor*inColor,1.0)/d2,0,1);
    // outColor=cos0*vec4(ubo.lightColor*inColor,1.0)/d2;
    // outColor=texture(texSampler,fragTexCoord);
    // outColor=cos0*vec4(ubo.lightColor*texture(texSampler,fragTexCoord).xyz,1.0)/d2;
    // outColor=0.5*cos0*vec4(ubo.lightColor*getTexture(texSampler,fragTexCoord),1.0)/d2+0.5*cos1*vec4(ubo.lightColor*getTexture(texSampler,fragTexCoord),1.0)/d2+0.1*vec4(0.1,0.1,0.1,0.5);




    // outColor=vec4(inColor,1.0)/d2;
    
}

// void main() 
// {
//     // outColor = texture(texSampler, fragTexCoord);

//     float shadow=shadowFactor(inShadowCoord/inShadowCoord.w);

//     vec3 lightVec=(ubo.lightPos-inPosition);
//     float d2reci=1.0/dot(lightVec,lightVec);

//     vec3 lightVec_normalized=normalize(lightVec);
//     vec3 eyeVec_normalized=normalize(ubo.eyePos-inPosition);
//     vec3 halfVec=gethalfvec(lightVec_normalized,eyeVec_normalized);
    
    
//     float cos0=clamp(dot(halfVec,inNormal),0,1);
//     float cos1=clamp(dot(inNormal,lightVec_normalized),0,1);

//     vec3 kd_texture=ubo.kd*getTexture(texSampler,fragTexCoord);

//     vec3 diffuse=kd_texture*ubo.lightColor*cos1*d2reci;
//     vec3 specular=ubo.ks*ubo.lightColor*pow(cos0,30)*d2reci;
//     vec3 ambient=vec3(0.02,0.02,0.02)*kd_texture;
    
//     if(modelId_flat==0) outColor=0.4*vec4(1,1,1,1);
//     else outColor=vec4((specular+diffuse+ambient)*shadow,1.0);

    
//     // outColor=clamp(pow(cos0,ubo.specFactor)*vec4(ubo.lightColor*inColor,1.0)/d2,0,1);
//     // outColor=cos0*vec4(ubo.lightColor*inColor,1.0)/d2;
//     // outColor=texture(texSampler,fragTexCoord);
//     // outColor=cos0*vec4(ubo.lightColor*texture(texSampler,fragTexCoord).xyz,1.0)/d2;
//     // outColor=0.5*cos0*vec4(ubo.lightColor*getTexture(texSampler,fragTexCoord),1.0)/d2+0.5*cos1*vec4(ubo.lightColor*getTexture(texSampler,fragTexCoord),1.0)/d2+0.1*vec4(0.1,0.1,0.1,0.5);




//     // outColor=vec4(inColor,1.0)/d2;
    
// }