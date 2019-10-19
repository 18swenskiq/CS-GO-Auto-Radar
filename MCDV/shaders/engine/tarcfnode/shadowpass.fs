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
uniform vec3 secondary;
uniform float spread_factor;

// for reprojection
uniform mat4 projection;
uniform mat4 view;

// pixel perfect noise mapping
uniform vec2 noiseScale;
uniform vec2 noiseOffset;

float trace(vec3 start, vec3 dir, float mxDist){
	for(int i = 0; i < 128; i++){
		
		vec3 sample = start + (normalize(dir) * (i/128.0) * mxDist);

		vec4 offset = vec4(sample, 1.0);
		offset = projection * view * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float depth = texture(gbuffer_position, offset.xy).y;
		if(depth > 9900) continue;

		if(depth >= sample.y + 0.01) 
		{
			return 1.0;
		}
	}

	return 0.0;
}

void main()
{
	// Get samples from position and normal buffers
	vec4 s_position = texture(gbuffer_position, TexCoords);
	vec4 s_normal = texture(gbuffer_normal, TexCoords);

	vec3 s_noise = texture(ssaoRotations, (TexCoords * noiseScale) + noiseOffset).rgb;
	
	FragColor = vec4(trace(s_position.xyz, sun_dir + (secondary * spread_factor), 1024.0) * accum_divisor, 0, 0, 1);
}