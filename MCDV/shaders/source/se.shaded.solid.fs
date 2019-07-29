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
	float diffuse = max(dot(Normal, normalize(vec3(-0.5,1,-0.3))), 0.0);
	FragColor = vec4(diffuse, diffuse, diffuse, 1.0) * color;
}