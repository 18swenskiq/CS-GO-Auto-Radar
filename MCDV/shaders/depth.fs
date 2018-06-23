#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float test;

uniform float HEIGHT_MIN;
uniform float HEIGHT_MAX;

in vec3 FragPos;
in float Depth;
in float Alpha;


void main()
{
	float height = (Depth - HEIGHT_MIN) / HEIGHT_MAX;

	FragColor = vec4(height, height, height, Alpha);
}