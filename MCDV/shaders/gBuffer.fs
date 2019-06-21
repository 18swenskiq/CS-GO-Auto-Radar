#version 330 core

in vec3 FragPos;
in vec3 Normal;
uniform uint Info;
uniform vec2 origin;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out uint gInfo;
layout (location = 3) out vec2 gOrigin;

void main()
{
	gPosition = FragPos;
	gNormal = normalize(Normal);
	gInfo = Info;
	gOrigin = origin;
}