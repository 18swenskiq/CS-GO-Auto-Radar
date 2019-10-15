#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;
uniform sampler2D MainTex1;
uniform sampler2D Mask;
uniform float factor;
uniform int maskChannelID;

// Frag main
void main()
{
	vec4 s_a = vec4(texture(MainTex, TexCoords));
	vec4 s_b = vec4(texture(MainTex1, TexCoords));
	float s_mask = texture(Mask, TexCoords)[maskChannelID];
	FragColor = vec4(mix(s_a.rgb, s_b.rgb, s_mask * s_b[3] * factor), clamp(s_a[3] + (s_b[3] * factor), 0, 1));
}