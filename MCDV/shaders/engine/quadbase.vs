#version 330 core
layout (location = 0) in vec4 aData;

out vec2 TexCoords;

void main()
{
	TexCoords = aData.zw;
	gl_Position = vec4(aData.xy, 0.0, 1.0);
}