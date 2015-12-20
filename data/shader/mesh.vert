#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 3) in vec3 inColor;

struct Instance
{
	mat4 model;
	vec4 color;
};

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
} ubo;

layout (binding = 1) uniform UBOInst
{
	Instance instance[343];
} uboinstance;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outEyePos;
layout (location = 3) out vec3 outLightVec;

void main() 
{
	outNormal = inNormal;
	outColor = inColor;
	outColor = uboinstance.instance[gl_InstanceID].color.rgb;
	mat4 modelView = ubo.view * uboinstance.instance[gl_InstanceID].model;	
	gl_Position = ubo.projection * modelView * vec4(inPos.xyz, 1.0);
	outEyePos = (gl_Position).xyz;
	vec4 lightPos = vec4(0.0, 0.0, 0.0, 1.0) * modelView;
	outLightVec = normalize(lightPos.xyz - outEyePos);	
}