#version 330

in vec3 fs_color;

out vec4 out_Color;

void main(void)
{
	out_Color = vec4(fs_color, 1.0f);
}