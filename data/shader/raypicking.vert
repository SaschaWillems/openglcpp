#version 410

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
	int selected;
} ubo;

layout (location = 0) out vec3 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main() 
{
	vec3[3] colors;
	colors[0] = vec3(1.0f, 0.0f, 0.0f);
	colors[1] = vec3(0.0f, 1.0f, 0.0f);
	colors[2] = vec3(0.0f, 0.0f, 1.0f);
	outColor = colors[gl_VertexID % 3];
	if (gl_VertexID / 3 == ubo.selected)
	{
		outColor = vec3(1.0f);
	}
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
}
