#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;

// Frag main
void main()
{
	vec4 s_tex = texture(MainTex, TexCoords);
	FragColor = vec4(vec3(1,1,1) - s_tex.rgb, s_tex.a);
}