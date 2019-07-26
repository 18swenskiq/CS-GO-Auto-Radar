#version 330 core

// Vert shader ins
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 Origin;

// GBuffer channels (RGB16F)
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gOrigin;

// Frag main
void main()
{
	gPosition = FragPos;
	gNormal = Normal;
	gOrigin = Origin;
}