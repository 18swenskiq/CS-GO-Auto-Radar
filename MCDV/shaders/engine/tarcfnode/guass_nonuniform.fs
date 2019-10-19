#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;
uniform sampler2D Modulate;
uniform vec2 direction;

// Frag main
void main()
{
	vec2 resolution = vec2(textureSize(MainTex, 0));

	vec4 color = vec4(0.0);
	vec2 off1 = vec2(1.411764705882353) * direction;
	vec2 off2 = vec2(3.2941176470588234) * direction;
	vec2 off3 = vec2(5.176470588235294) * direction;

	color += texture(MainTex, TexCoords) * 0.1964825501511404;
	color += texture(MainTex, TexCoords + (off1 / resolution)) * 0.2969069646728344;
	color += texture(MainTex, TexCoords - (off1 / resolution)) * 0.2969069646728344;
	color += texture(MainTex, TexCoords + (off2 / resolution)) * 0.09447039785044732;
	color += texture(MainTex, TexCoords - (off2 / resolution)) * 0.09447039785044732;
	color += texture(MainTex, TexCoords + (off3 / resolution)) * 0.010381362401148057;
	color += texture(MainTex, TexCoords - (off3 / resolution)) * 0.010381362401148057;

	FragColor = color;
}