#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float test;

uniform float HEIGHT_MIN;
uniform float HEIGHT_MAX;

in vec3 FragPos;
in float Depth;
in float Alpha;

// Simple remap from-to range.
float remap(float value, float low1, float high1, float low2, float high2)
{
	return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

void main()
{
	float height = pow(remap(FragPos.y, HEIGHT_MIN, HEIGHT_MAX, 0, 1), 2.2);

	FragColor = vec4(0, height, 1, 1.0);
}