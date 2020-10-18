#version 330 core

in vec3 sh_FragPos;
in vec3 sh_Normal;
in vec2 sh_Origin;

uniform vec2 in_Origin;

layout (location = 0) out vec3 out_Position;
layout (location = 1) out vec3 out_Normal;
layout (location = 2) out vec2 out_Origin;

void main()
{
	out_Position = sh_FragPos * 0.001;
	out_Normal = -normalize( sh_Normal ) * vec3(0.5) + vec3(0.5);
	out_Origin = (sh_Origin + in_Origin) * 0.001;
}
