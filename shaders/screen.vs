#version 330 core
layout (location = 0) in vec4 va_Pos;

out vec2 sh_UV;

void main()
{
	sh_UV = va_Pos.zw;
	gl_Position = vec4(va_Pos.xy, 0.0, 1.0);
}
