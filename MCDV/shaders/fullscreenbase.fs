#version 330 core
// I sincerily apologize to anyone reading this mess of a shader... -Terri00 21 6 19

//                                         OPENGL
// ____________________________________________________________________________________________
in vec2 TexCoords;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out float hData;
//layout (location = 2) out uint aoBuffer;

//                                        UNIFORMS
// Vector Information _________________________________________________________________________
//    ( A bunch of vectors that give you the location of different entities )
uniform vec3 bounds_NWU;	// North-West-Upper coordinate of the playspace (worldspace)
uniform vec3 bounds_SEL;	// South-East-Lower coordinate of the playspace (worldspace)

//                                     SAMPLER UNIFORMS
// Image Inputs _______________________________________________________________________________
uniform sampler2D tex_gradient;
uniform sampler2D tex_modulate;
uniform sampler2D gbuffer_position;
uniform sampler2D gbuffer_origin;
uniform sampler2D gbuffer_clean_position;
uniform sampler2D gbuffer_normal;
uniform usampler2D gbuffer_info;
uniform usampler2D umask_playspace;
uniform usampler2D umask_objectives;
uniform usampler2D umask_buyzone;

uniform vec3 samples[256];
uniform sampler2D ssaoRotations;
uniform float ssaoScale;
uniform int mssascale;
uniform mat4 projection;
uniform mat4 view;

const vec2 noiseScale = vec2(1024.0/256.0, 1024.0/256.0);

uniform vec4 color_objective;
uniform vec4 color_buyzone;
uniform vec4 color_cover;
uniform vec4 color_cover2;
uniform vec4 color_ao;

uniform float blend_objective_stripes;
uniform float blend_ao;

//                                       SHADER HELPERS
// ____________________________________________________________________________________________
// --------------------------------------- Blend modes ----------------------------------------

float lerp(float a, float b, float w)
{
	return a + w*(b-a);
}

vec3 lerp(vec3 a, vec3 b, float w)
{
  return a + w*(b-a);
}

vec4 lerp(vec4 a, vec4 b, float w)
{
	return a + w*(b-a);
}

vec4 blend_normal(vec4 a, vec4 b, float s)
{
	return vec4(lerp(a.rgb, b.rgb, b.a * s), a.a + (b.a * s));
}

vec4 blend_add(vec4 a, vec4 b, float s)
{
	return vec4(a.rgb + (b.rgb * s), a.a);
}

// ------------------------------------------ maths -------------------------------------------
float remap(float value, float low1, float high1, float low2, float high2)
{
	return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

//                                       SHADER PROGRAM
// ____________________________________________________________________________________________
//     ( Write all your shader code & functions here )

vec4 sample_gradient(float height)
{
	return vec4(texture(tex_gradient, vec2(remap(height, bounds_SEL.y, bounds_NWU.y, 0, 1), 0)));
}

float kernel_filter_glow(sampler2D sampler, int channelID, int sample_size, int inverse)
{
	vec2 pixel_size = 1.0 / vec2(textureSize(sampler, 0));

	float sT = 0;
	int sample_double = sample_size * 2;

	// Process kernel
	for(int x = 0; x <= sample_double; x++){
		for(int y = 0; y <= sample_double; y++){
			if(inverse == 0)
			sT += texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y))[channelID];
			else sT += 1 - texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y))[channelID];
		}
	}

	sT /= (sample_double * sample_double);

	return sT;
}

// Given a 0-1 mask, return an outline drawn around that mask
float kernel_filter_outline(sampler2D sampler, int channelID, int sample_size)
{
	vec2 pixel_size = 1.0 / vec2(textureSize(sampler, 0));

	float sT = 0;
	int sample_double = sample_size * 2;
	
	// Process kernel
	for(int x = 0; x <= sample_double; x++){
		for(int y = 0; y <= sample_double; y++){
			sT += //texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y))[channelID];
			(sample_size - min(length(vec2(-sample_size + x, -sample_size + y)), sample_size)) * 
			texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y))[channelID];
		}
	}

	return max(min(sT, 1) - texture(sampler, TexCoords)[channelID], 0);
}

float kernel_filter_glow(usampler2D sampler, int sample_size, int inverse)
{
	vec2 pixel_size = 1.0 / vec2(textureSize(sampler, 0));

	uint sT = 0U;
	int sample_double = sample_size * 2;

	// Process kernel
	for(int x = 0; x <= sample_double; x++){
		for(int y = 0; y <= sample_double; y++){
			if(inverse == 0)
			sT += texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y)).r;
			else sT += 1U - texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y)).r;
		}
	}
	float r = float(sT) / (sample_double * sample_double);
	return r * r;
}

float _kernel_filter_glow(usampler2D sampler, usampler2D samplerMask, int sample_size, int inverse)
{
	vec2 pixel_size = 1.0 / vec2(textureSize(sampler, 0));

	uint sT = 0U;
	int sample_double = sample_size * 2;

	// Process kernel
	for(int x = 0; x <= sample_double; x++){
		for(int y = 0; y <= sample_double; y++){
			if(inverse == 0)
			sT += (texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y)).r) * texture(samplerMask, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y)).r;
			else sT += (1U - texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y)).r) * texture(samplerMask, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y)).r;
		}
	}
	float r = float(sT) / (sample_double * sample_double);
	return r * r;
}

// Given a 0-1 mask, return an outline drawn around that mask
float kernel_filter_outline(usampler2D sampler, int sample_size)
{
	vec2 pixel_size = 1.0 / vec2(textureSize(sampler, 0));

	float sT = 0;
	int sample_double = sample_size * 2;
	
	// Process kernel
	for(int x = 0; x <= sample_double; x++){
		for(int y = 0; y <= sample_double; y++){
			sT += 
			(sample_size - min(length(vec2(-sample_size + x, -sample_size + y)), sample_size)) * 
			float(texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y)).r);
		}
	}

	return float(max(min(sT, 1U) - texture(sampler, TexCoords).r, 0U));
}

// Given a 0-1 mask, return an outline drawn around that mask
float _kernel_filter_outline(usampler2D sampler, usampler2D samplerMask, int sample_size)
{
	vec2 pixel_size = 1.0 / vec2(textureSize(sampler, 0));

	float sT = 0;
	int sample_double = sample_size * 2;
	
	// Process kernel
	for(int x = 0; x <= sample_double; x++){
		for(int y = 0; y <= sample_double; y++){
			sT += 
			(sample_size - min(length(vec2(-sample_size + x, -sample_size + y)), sample_size)) * 
			float(texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y)).r) * 
			texture(samplerMask, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y)).r;
		}
	}

	return float(max(min(sT, 1U) - (texture(sampler, TexCoords).r * texture(samplerMask, TexCoords).r), 0U));
}

void main()
{
	//vec4 s_background = texture(tex_background, TexCoords);
	vec4 final = vec4(0,0,0,0);

	vec4 s_position = texture(gbuffer_position, TexCoords);
	vec4 s_modulate_1_5 = texture(tex_modulate, TexCoords * 1.5);
	vec4 s_modulate = texture(tex_modulate, TexCoords);
	vec4 s_position_clean = texture(gbuffer_clean_position, TexCoords);
	float htest = remap(s_position.y, bounds_SEL.y, bounds_NWU.y, 0, 1);

	uint s_um_playspace = texture(umask_playspace, TexCoords).r;
	
	float m_playspace_clean = float(((s_um_playspace >> 0) & 0x1U));

	float m_objectives = float(texture(umask_objectives, TexCoords).r);
	float m_buyzones = float(texture(umask_buyzone, TexCoords).r);
	uint s_info = texture(gbuffer_info, TexCoords).r;
	float m_playspace =	 float(((s_um_playspace >> 0) & 0x1U) | ((s_info >> 1) & 0x1U));

	final = blend_normal(final, 
	sample_gradient(
		lerp(s_position_clean.y, s_position.y, clamp((1 - s_modulate.r) + (float((s_info >> 1) & 0x1U) - m_playspace_clean), 0, 1))
	), m_playspace);

	vec2 sloc = texture(gbuffer_origin, TexCoords).xy;
	vec4 originSample = vec4(sloc.x, 0, sloc.y, 1);
	originSample = projection * view * originSample;
	originSample.xyz /= originSample.w;
	originSample.xyz = originSample.xyz * 0.5 + 0.5;

	//float sh = (s_position.y - texture(gbuffer_clean_position, originSample.xy).y) / 196.0;

	//FragColor = vec4(sh, sh, sh, 1);

	final = blend_normal(
	final, 
	lerp(color_cover, color_cover2, 
		float((s_info >> 7) & 0x1U) * m_playspace * (1 - ((s_position.y - texture(gbuffer_clean_position, originSample.xy).y) / 196))), 
		float((s_info >> 7) & 0x1U) * m_playspace);

	vec4 s_normal = texture(gbuffer_normal, TexCoords);
	vec3 randVec = texture(ssaoRotations, TexCoords * noiseScale).rgb;

	vec3 tangent = normalize(randVec - s_normal.rgb * dot(randVec, s_normal.rgb));
	vec3 bitangent = cross(s_normal.rgb, tangent);
	mat3 TBN = mat3(tangent, bitangent, s_normal.rgb);

	hData = s_position.y;

	float occlusion = 0.0;
	for(int i = 0; i < 256; i++)
	{
		vec3 sample = TBN * samples[i];
		sample = s_position.xyz + sample * ssaoScale;

		vec4 offset = vec4(sample, 1.0);
		offset = projection * view * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sDepth = texture(gbuffer_position, offset.xy).y;

		occlusion += (sDepth >= sample.y + 6 ? 1.0 : 0.0);
	}

	final = blend_normal(final, color_ao, (occlusion / 200) * m_playspace * blend_ao);

	//aoBuffer = occlusion / 200;

	final = blend_normal(final, color_objective,																// Objectives
		(
		(_kernel_filter_glow(umask_objectives, umask_playspace, 13 * mssascale, 1))
		* m_objectives
		* ( 1 - float((s_info >> 7) & 0x1U))
		)
		+ 
		(
		_kernel_filter_outline(umask_objectives, umask_playspace, 3 * mssascale) * 0.9
		* ( 1 - float((s_info >> 7) & 0x1U)) * clamp(s_modulate_1_5.r + blend_objective_stripes, 0, 1)
		)
		+
		(
		(_kernel_filter_glow(umask_objectives, umask_playspace, 13 * mssascale, 0))
		* ( 1 - m_objectives )
		* ( 1 - float((s_info >> 7) & 0x1U))
		)
		+ (float(texture(umask_objectives, TexCoords).r * texture(umask_playspace, TexCoords).r) * 0.08)
		);
	
	final = blend_normal(final, color_buyzone,																// Buyzone
		(
		(_kernel_filter_glow(umask_buyzone, umask_playspace, 13 * mssascale, 1))
		* m_buyzones
		* ( 1 - float((s_info >> 7) & 0x1U))
		)
		+ 
		(
		_kernel_filter_outline(umask_buyzone, umask_playspace, 3 * mssascale) * 0.9
		* ( 1 - float((s_info >> 7) & 0x1U))
		)
		+
		(
		(_kernel_filter_glow(umask_buyzone, umask_playspace, 13 * mssascale, 0))
		* ( 1 - m_buyzones )
		* ( 1 - float((s_info >> 7) & 0x1U))
		)
		+ (float(texture(umask_buyzone, TexCoords).r * texture(umask_playspace, TexCoords).r) * 0.07)
		);

	FragColor = final;
}