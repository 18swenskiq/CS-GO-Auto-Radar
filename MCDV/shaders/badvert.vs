#version 330 core
layout (location = 0) in vec4 vert;
out vec2 TexCoords;

uniform mat4 model;

void main()
{
	gl_Position = model * vec4(vert.xy, 0.0, 1.0)
	TexCoords = vert.zw;
}