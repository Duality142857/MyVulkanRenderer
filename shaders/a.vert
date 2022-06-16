#version 450
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

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragTexCoord;





void main()
{
    mat4 transformMat=ubo.proj*ubo.view*ubo.model;
    gl_Position=transformMat*vec4(inPosition,1.f);
    vec4 worldPos=ubo.model*vec4(inPosition,1.0f);
    float w_reci=1.0/worldPos.w;
    fragPosition=worldPos.xyz*w_reci;
    fragColor=inColor;
    fragNormal=(transformMat*vec4(inNormal,0.f)).xyz;
    fragTexCoord=inTexCoord;
}