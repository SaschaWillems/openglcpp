/*
This code is licensed under the Mozilla Public License Version 2.0 (http://opensource.org/licenses/MPL-2.0)
© 2014 by Sascha Willems - http://www.saschawillems.de

This geometry shader takes a single GL_POINT as input and generates triangles for a complete circle.
*/

#version 330

layout (points) in;
layout (triangle_strip, max_vertices=256) out;

in vec3 vs_color[];
out vec3 fs_color;

uniform float inRadius;
uniform int inNumDivisions;

void main()
{
	// Generate a circle based on points center position
	for (int i=0; i < inNumDivisions; i++) {
		float degInRad = i*3.14159f/180.0f * (360.0f/inNumDivisions);
		float degInRadB = (i+1)*3.14159f/180.0f * (360.0f/inNumDivisions);

		// For each subdivision we create a single triangle
		gl_Position = gl_in[0].gl_Position;
		fs_color = vs_color[0];
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4(inRadius*cos(degInRad), inRadius*sin(degInRad), 0.0f, 0.0f);
		fs_color = vs_color[0];
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4(inRadius*cos(degInRadB), inRadius*sin(degInRadB), 0.0f, 0.0f);
		fs_color = vs_color[0];
		EmitVertex();

		EndPrimitive();
	}
}