#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;
uniform sampler2D MainTex1;
uniform float factor;

// Frag main
void main()
{
	vec4 s_a = vec4(texture(MainTex, TexCoords));
	vec4 s_b = vec4(texture(MainTex1, TexCoords));
	FragColor = vec4(s_a.rgb * s_b.rgb, 1);
}