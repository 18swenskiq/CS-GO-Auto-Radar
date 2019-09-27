#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;
uniform float edge;

// Frag main
void main()
{
	vec4 s_tex = texture(MainTex, TexCoords);
	FragColor = vec4(step(edge, s_tex.r), step(edge, s_tex.g), step(edge, s_tex.b), s_tex.a);
}