#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;
uniform sampler2D MainTex1;
uniform sampler2D Mask;
uniform float factor;

// Frag main
void main()
{
	vec4 s_a = vec4(texture(MainTex, TexCoords));
	vec4 s_b = vec4(texture(MainTex1, TexCoords));
	float s_mask = texture(Mask, TexCoords)[0];
	FragColor = mix(s_a, s_b, s_mask * s_b[3] * factor);
}