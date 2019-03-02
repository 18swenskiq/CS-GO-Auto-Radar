#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 color;
uniform float alpha;
uniform sampler2D text;

void main()
{
	vec4 sample = texture(text, TexCoords);

	FragColor = vec4(sample.r, sample.g, sample.b, sample.a * alpha);
	//FragColor = vec4(texture(text, TexCoords).r, 0.0, 0.0, 1.0);
}