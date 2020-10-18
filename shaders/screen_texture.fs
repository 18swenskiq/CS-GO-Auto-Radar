#version 330 core
out vec4 FragColor;

uniform sampler2D in_Sampler;

in vec2 sh_UV;

void main()
{
	FragColor = texture( in_Sampler, sh_UV );
}
