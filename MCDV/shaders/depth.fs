#version 330 core
out vec4 FragColor;

uniform vec3 color;

uniform float HEIGHT_MIN;
uniform float HEIGHT_MAX;
uniform float write_playable;
uniform float write_cover;

in vec3 FragPos;
in float Depth;

// Simple remap from-to range.
float remap(float value, float low1, float high1, float low2, float high2)
{
	return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

void main()
{
	float height = remap(FragPos.y, HEIGHT_MIN, HEIGHT_MAX, 0, 1);// pow(, 2.2);

	FragColor = vec4(write_playable, height, write_cover, 1);
}