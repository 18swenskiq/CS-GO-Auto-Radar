#version 330 core
in vec2 TexCoords;
layout (location = 0) out vec4 FragColor;

uniform vec3 samples[64];
uniform float ssaoScale;
uniform float bias;
uniform float blendFac;

// Random rotations
uniform sampler2D ssaoRotations;
uniform sampler2D gbuffer_position;
uniform sampler2D gbuffer_normal;

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

	vec3 randVec = texture(ssaoRotations, (TexCoords * noiseScale) + noiseOffset).rgb;
	vec3 tangent = normalize(randVec - s_normal.rgb * dot(randVec, s_normal.rgb));
	vec3 bitangent = cross(s_normal.rgb, tangent);
	mat3 TBN = mat3(tangent, bitangent, s_normal.rgb);

	float occlusion = 0.0;
	for(int i = 0; i < 64; i++)
	{
		vec3 sample = TBN * samples[i];
		sample = s_position.xyz + (sample * ssaoScale);

		vec4 offset = vec4(sample, 1.0);
		offset = projection * view * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float depth = texture(gbuffer_position, offset.xy).y;

		occlusion += (depth >= sample.y + bias ? 1.0 : 0.0);
	}
	
	occlusion /= 60;

	FragColor = vec4(1, 0, 0, occlusion * blendFac);
}