#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;

uniform vec3 swang;

// Frag main
void main()
{
	FragColor = vec4(TexCoords.xy, swang.x, 1);
}