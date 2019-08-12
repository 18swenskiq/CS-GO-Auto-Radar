#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;
uniform sampler2D Gradient;
uniform float s_minimum;
uniform float s_maximum;
uniform int channelID;

float remap(float value, float low1, float high1, float low2, float high2)
{
	return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

// Frag main
void main()
{
	float sample = remap(vec4(texture(MainTex, TexCoords))[channelID], s_minimum, s_maximum, 0, 1);
	vec4 grad_color = texture(Gradient, vec2(sample, 0.5));
	
	FragColor = grad_color;
}