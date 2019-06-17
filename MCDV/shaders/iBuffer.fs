#version 330 core

in vec3 FragPos;
in vec3 Normal;

uniform uint srcChr;

//layout (location = 0) out vec3 gPosition;

layout (location = 0) out uint gInfo;

void main()
{
	//gPosition = FragPos;
	//gNormal = vec3(1,0,0);//normalize(Normal);
	gInfo = srcChr;
}