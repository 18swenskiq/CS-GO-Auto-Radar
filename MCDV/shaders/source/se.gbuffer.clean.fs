#version 330 core

// Vert shader ins
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 Origin;

// GBuffer channels (RGB16F)
layout (location = 0) out vec3 gPosition;

// Frag main
void main()
{
	gPosition = vec3(FragPos.y, FragPos.y, FragPos.y);
}