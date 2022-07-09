#version 460
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
} ubo_scene;

//transform from [-1,1] to [0,1]
const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in int modelId;

layout(location = 5) in float rotationAngle;
layout(location = 6) in vec3 rotationAxis;
layout(location = 7) in vec3 scaling;
layout(location = 8) in vec3 translation;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout (location = 3) flat out int modelId_flat;
layout(location =4 ) out vec4 fragShadowCoord;
layout(location=5) out vec3 fragViewVec_normalized;
layout(location=6) out vec3 fragLightVec;

mat4 translate(vec3 tv)
{
    return mat4(1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                tv.x,tv.y,tv.z,1);
}


void main()
{

    mat4 models[3];
    // models[2]=translate(vec3(gl_InstanceIndex*7,-2,0)-vec3(4,0,0))*ubo.model;
    // models[1]=translate(vec3(gl_InstanceIndex*7,0,0))*ubo.model;
    // models[0]=translate(ubo_scene.lightPos+vec3(4,0,0));
    models[0]=translate(ubo_scene.lightPos);

    models[1]=mat4(1,0,0,0,0,1,0,0,0,0,1,0,translation.x,translation.y,translation.z,1)*ubo_scene.model;
    models[2]=mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,-5,1);
    // models[2]=mat4(1,0,0,0,
    //                 0,1,0,0,
    //                 0,0,1,0,
    //                 0,0,0,1);


    // mat4 model=translate(vec3(gl_InstanceIndex*7,-2,0)-vec3(4,0,0))*ubo.model;
    modelId_flat=modelId;

    // if(modelId==0) model=translate(ubo.lightPos+vec3(4,0,0));
    // if(modelId==0) model=translate(vec3(gl_InstanceIndex*2,0,0));

    mat4 transformMat=ubo_scene.proj*ubo_scene.view*models[modelId];
    gl_Position=transformMat*vec4(inPosition,1.f);

    vec4 worldPos=models[modelId]*vec4(inPosition,1.0f);

    float w_reci=1.0/worldPos.w;
    fragPosition=worldPos.xyz*w_reci;

    fragNormal=(models[modelId]*vec4(inNormal,0.f)).xyz;
    
    fragTexCoord=inTexCoord;

    fragShadowCoord=biasMat*ubo_scene.lightMVP*models[modelId]*vec4(inPosition,1.0);

    fragViewVec_normalized=normalize(ubo_scene.eyePos-worldPos.xyz);
    fragLightVec=ubo_scene.lightPos-worldPos.xyz;
}