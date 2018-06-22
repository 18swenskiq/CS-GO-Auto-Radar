#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out float Depth;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;

	gl_Position = projection * view * model * vec4(aPos, 1.0);
	//Depth = gl_Position.z / 100.0;
	Depth = FragPos.y / 10.0;
}