#version 400
uniform vec4 inColor;
uniform sampler2D inTex;
out vec4 color;

void main () {
	color = vec4(texture(inTex, gl_PointCoord) * inColor);
}
