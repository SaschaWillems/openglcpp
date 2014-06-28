#version 330

in vec3 in_Position;
in vec3 in_Color;

out vec3 vs_color;

void main(void)
{
    gl_Position = vec4(in_Position.xyz, 1.0);
    vs_color = in_Color;
}