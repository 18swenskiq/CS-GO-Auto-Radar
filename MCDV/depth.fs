#version 330 core
out vec4 FragColor;

uniform vec3 color;

in vec3 FragPos;
in float Depth;

void main()
{
	FragColor = vec4(Depth, Depth, Depth, 1.0);
}