#version 330 core
layout (location = 0) in vec2 vert;
out vec2 TexCoords;

void main()
{
	gl_Position = vec4(vert.xy, 0.0, 1.0);
	TexCoords = (vert.xy + vec2(1, 1)) * 0.5;
}