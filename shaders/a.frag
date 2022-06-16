#version 450

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
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location=3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec3 gethalfvec(vec3 a, vec3 b)
{
    return normalize(a+b);
}

void main() {
    // outColor = texture(texSampler, fragTexCoord);

    vec3 lightVec=(ubo.lightPos-inPosition);
    float d2=dot(lightVec,lightVec);

    vec3 lightVec_normalized=normalize(lightVec);
    vec3 eyeVec_normalized=normalize(ubo.eyePos-inPosition);
    vec3 halfVec=gethalfvec(lightVec_normalized,eyeVec_normalized);
    
    
    float cos0=dot(halfVec,inNormal);
    
    // outColor=clamp(pow(cos0,ubo.specFactor)*vec4(ubo.lightColor*inColor,1.0)/d2,0,1);
    // outColor=cos0*vec4(ubo.lightColor*inColor,1.0)/d2;
    // outColor=texture(texSampler,fragTexCoord);
    outColor=cos0*vec4(ubo.lightColor*texture(texSampler,fragTexCoord).xyz,1.0)/d2;



    // outColor=vec4(inColor,1.0)/d2;
    
}