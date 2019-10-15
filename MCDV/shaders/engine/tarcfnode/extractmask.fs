#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;

// Frag main
void main()
{
	vec4 s_tex = texture(MainTex, TexCoords);
	float pColor = 0.0;
	if(s_tex.r > 0) pColor = 1.0;

	FragColor = vec4(pColor, pColor, pColor, s_tex.a);
}