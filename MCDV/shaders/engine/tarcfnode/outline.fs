#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

uniform sampler2D MainTex;

uniform float width;

float kernel_filter_outline(sampler2D sampler, int channelID, int sample_size)
{
	vec2 pixel_size = 1.0 / vec2(textureSize(sampler, 0));

	int sample_mx = min(sample_size, 16);

	float sT = 0;
	int sample_double = sample_mx * 2;
	
	// Process kernel
	for(int x = 0; x <= sample_double; x++){
		for(int y = 0; y <= sample_double; y++){
			sT += step(0.5, texture(sampler, TexCoords + vec2((-sample_mx + x) * pixel_size.x, (-sample_mx + y) * pixel_size.y))[channelID]);
		}
	}

	return max(min(sT, 1) - step(0.5, texture(sampler, TexCoords)[channelID]), 0);
}


// Frag main
void main()
{
	float s_kernel = kernel_filter_outline(MainTex, 0, 3);
	FragColor = vec4(s_kernel, s_kernel, s_kernel, 1);
}