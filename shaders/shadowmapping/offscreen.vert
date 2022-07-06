#version 450

// layout (location = 0) in vec3 inPos;
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in int modelId;

layout (binding = 0) uniform UBO 
{
	mat4 depthMVP;
	mat4 sceneModel;
} ubo;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

 
void main()
{
	if(modelId==2) gl_Position=ubo.depthMVP*mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,-5,1)*vec4(inPosition,1.0);
	else if(modelId==1) gl_Position =  ubo.depthMVP*ubo.sceneModel * vec4(inPosition, 1.0);
}