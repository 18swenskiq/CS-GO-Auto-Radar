#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;
uniform float NormalizeScale;

// Frag main
void main()
{
	FragColor = vec4(texture(MainTex, TexCoords).xyz * NormalizeScale, 1.0);
}