#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Color
uniform vec4 color;

// Frag main
void main()
{
	float diffuse = (dot(Normal, normalize(vec3(-0.5,1,-0.3))) * 0.5) + 0.5;

	float modx = step(0.5, 1 - mod(FragPos.x, 32.0)) * (1.0 - abs(Normal.x));
	float mody = step(0.5, 1 - mod(FragPos.y, 32.0)) * (1.0 - abs(Normal.y));
	float modz = step(0.5, 1 - mod(FragPos.z, 32.0)) * (1.0 - abs(Normal.z));
	float addr = clamp(modx+mody+modz, 0, 1.0);

	FragColor = vec4(diffuse, diffuse, diffuse, 1.0) * color + (vec4(0.1, 0.1, 0.1, 0.0)*addr);
}