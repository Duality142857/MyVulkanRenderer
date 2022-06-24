#version 460
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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in int modelId;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 kd;
layout(location = 2) out vec3 ks;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec2 fragTexCoord;
layout (location = 5) flat out int modelId_flat;

mat4 translate(vec3 tv)
{
    return mat4(1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                tv.x,tv.y,tv.z,1);
}


void main()
{

    mat4 models[2];
    // models[2]=translate(vec3(gl_InstanceIndex*7,-2,0)-vec3(4,0,0))*ubo.model;
    models[1]=translate(vec3(gl_InstanceIndex*7,-2,0)-vec3(4,0,0))*ubo.model;
    models[0]=translate(ubo.lightPos+vec3(4,0,0));

    // mat4 model=translate(vec3(gl_InstanceIndex*7,-2,0)-vec3(4,0,0))*ubo.model;
    modelId_flat=modelId;

    // if(modelId==0) model=translate(ubo.lightPos+vec3(4,0,0));
    // if(modelId==0) model=translate(vec3(gl_InstanceIndex*2,0,0));

    mat4 transformMat=ubo.proj*ubo.view*models[modelId];
    gl_Position=transformMat*vec4(inPosition,1.f);

    vec4 worldPos=models[modelId]*vec4(inPosition,1.0f);

    float w_reci=1.0/worldPos.w;
    fragPosition=worldPos.xyz*w_reci;
    fragNormal=(models[modelId]*vec4(inNormal,0.f)).xyz;
    fragTexCoord=inTexCoord;
    kd=vec3(0.2,0.2,0.2)*inColor;
    ks=vec3(0.8,0.8,0.8)*inColor;
}