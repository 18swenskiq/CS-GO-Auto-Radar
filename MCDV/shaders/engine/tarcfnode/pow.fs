#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;
uniform float value;

// Frag main
void main()
{
	vec4 s_tex = texture(MainTex, TexCoords);
	FragColor = vec4(pow(s_tex.r, value), pow(s_tex.g, value), pow(s_tex.b, value), s_tex.a);
}