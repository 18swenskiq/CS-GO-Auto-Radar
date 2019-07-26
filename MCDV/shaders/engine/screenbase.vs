#version 330 core
layout (location = 0) in vec4 aData;

out vec2 TexCoords;
uniform mat4 model;

void main()
{
	TexCoords = aData.zw;
	gl_Position = model * vec4(aData.xy, 0.0, 1.0);
}