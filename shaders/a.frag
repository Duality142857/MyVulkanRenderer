#version 460

// layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    vec3 lightColor;
    vec3 eyePos;
    float specFactor;
} ubo;
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 kd;
layout(location = 2) in vec3 ks;
layout(location = 3) in vec3 inNormal;
layout(location=4) in vec2 fragTexCoord;
layout (location = 5) flat in int modelId_flat;


layout(location = 0) out vec4 outColor;

vec3 gethalfvec(vec3 a, vec3 b)
{
    return normalize(a+b);
}

vec3 getTexture(sampler2D sampl, vec2 tex)
{
    if(tex.x<-0.5) return kd;
    return texture(sampl,fragTexCoord).xyz;
}

void main() 
{
    // outColor = texture(texSampler, fragTexCoord);

    vec3 lightVec=(ubo.lightPos-inPosition);
    float d2reci=1.0/dot(lightVec,lightVec);

    vec3 lightVec_normalized=normalize(lightVec);
    vec3 eyeVec_normalized=normalize(ubo.eyePos-inPosition);
    vec3 halfVec=gethalfvec(lightVec_normalized,eyeVec_normalized);
    
    
    float cos0=clamp(dot(halfVec,inNormal),0,1);
    float cos1=clamp(dot(inNormal,lightVec_normalized),0,1);
    vec3 kd_texture=getTexture(texSampler,fragTexCoord);
    vec3 diffuse=kd_texture*ubo.lightColor*cos1*d2reci;
    vec3 specular=ks*ubo.lightColor*pow(cos0,30)*d2reci;
    vec3 ambient=vec3(0.02,0.02,0.02)*kd_texture;

    if(modelId_flat==0) outColor=0.4*vec4(1,1,1,1);
    else outColor=vec4(specular+diffuse+ambient,1.0);

    
    // outColor=clamp(pow(cos0,ubo.specFactor)*vec4(ubo.lightColor*inColor,1.0)/d2,0,1);
    // outColor=cos0*vec4(ubo.lightColor*inColor,1.0)/d2;
    // outColor=texture(texSampler,fragTexCoord);
    // outColor=cos0*vec4(ubo.lightColor*texture(texSampler,fragTexCoord).xyz,1.0)/d2;
    // outColor=0.5*cos0*vec4(ubo.lightColor*getTexture(texSampler,fragTexCoord),1.0)/d2+0.5*cos1*vec4(ubo.lightColor*getTexture(texSampler,fragTexCoord),1.0)/d2+0.1*vec4(0.1,0.1,0.1,0.5);




    // outColor=vec4(inColor,1.0)/d2;
    
}