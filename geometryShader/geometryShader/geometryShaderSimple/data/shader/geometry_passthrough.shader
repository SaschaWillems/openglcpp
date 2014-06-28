/*
This code is licensed under the Mozilla Public License Version 2.0 (http://opensource.org/licenses/MPL-2.0)
© 2014 by Sascha Willems - http://www.saschawillems.de

This is a simple pass through geometry shader that doesn't touch the input geometry at all.
*/

#version 330

layout (points) in;
layout (points) out;

in vec3 vs_color[];
out vec3 fs_color;

void main()
{
	for (int i = 0; i < gl_in.length(); i++)
		{
			gl_Position = gl_in[i].gl_Position;
			fs_color = vs_color[i];
			EmitVertex();
			EndPrimitive();
		}
}