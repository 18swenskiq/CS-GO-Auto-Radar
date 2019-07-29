#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform usampler2D MainTex;

// Frag main
void main()
{
	uint info = texture(MainTex, TexCoords).x;
	FragColor = vec4(float(info) / 255.0f, 0, 0, 1.0);
}