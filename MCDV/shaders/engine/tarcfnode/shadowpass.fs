#version 330 core
in vec2 TexCoords;
layout (location = 0) out vec4 FragColor;

uniform float ssaoScale;
uniform float bias;
uniform float blendFac;
uniform float accum_divisor;

// Random rotations
uniform sampler2D ssaoRotations;
uniform sampler2D gbuffer_position;
uniform sampler2D gbuffer_normal;
uniform vec3 sun_dir;

// for reprojection
uniform mat4 projection;
uniform mat4 view;

// pixel perfect noise mapping
uniform vec2 noiseScale;
uniform vec2 noiseOffset;

void main()
{
	// Get samples from position and normal buffers
	vec4 s_position = texture(gbuffer_position, TexCoords);
	vec4 s_normal = texture(gbuffer_normal, TexCoords);

	vec3 s_noise = texture(ssaoRotations, (TexCoords * noiseScale) + noiseOffset).rgb;

	float occlusion = 0.0;
	for(int i = 0; i < 128; i++)
	{
		vec3 sample = s_position.xyz + normalize((sun_dir * 1024.0) + (s_noise * pow((i/128.0), 2.0) * 128.0)) * (i/128.0) * 1024.0;

		vec4 offset = vec4(sample, 1.0);
		offset = projection * view * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float depth = texture(gbuffer_position, offset.xy).y;

		occlusion += (depth >= sample.y + bias ? 1.0 : 0.0);
	}
	
	occlusion /= accum_divisor;

	FragColor = vec4(1, 0, 0, clamp(occlusion * blendFac, 0, 1));
}