#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;
uniform float iter;

// Frag main
void main()
{
	vec2 pixel_size = 1.0 / vec2(textureSize(MainTex, 0));

	float s_n = texture(MainTex, TexCoords + vec2(0,   pixel_size.y))[0] * 1024.0;
	float s_ne = texture(MainTex, TexCoords + vec2(pixel_size.x, pixel_size.y))[0] * 1024.0;
	float s_e = texture(MainTex, TexCoords + vec2(pixel_size.x,	  0))[0] * 1024.0;
	float s_se = texture(MainTex, TexCoords + vec2(pixel_size.x, -pixel_size.y))[0] * 1024.0;
	float s_s = texture(MainTex, TexCoords + vec2(0,  -pixel_size.y))[0] * 1024.0;
	float s_sw = texture(MainTex, TexCoords + vec2(-pixel_size.x, -pixel_size.y))[0] * 1024.0;
	float s_w = texture(MainTex, TexCoords + vec2(-pixel_size.x,  0))[0] * 1024.0;
	float s_nw = texture(MainTex, TexCoords + vec2(-pixel_size.x, pixel_size.y))[0] * 1024.0;

	float v = max(clamp(s_n + s_s + s_e + s_w + s_ne + s_se + s_sw + s_nw, 0, 1) * iter, texture(MainTex, TexCoords)[0]);

	FragColor = vec4(v, v, v, 1);
}