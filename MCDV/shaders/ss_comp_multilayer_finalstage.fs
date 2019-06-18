#version 330 core
//                                         OPENGL
// ____________________________________________________________________________________________
in vec2 TexCoords;
out vec4 FragColor;

//                                     SAMPLER UNIFORMS
// Image Inputs _______________________________________________________________________________
uniform sampler2D tex_layer;
uniform sampler2D tex_background;
uniform float blend_outline;
uniform vec4 color_outline;
uniform int outline_width;

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

// ---------------------------------- kernel / trace filters ----------------------------------
// Given an 0-1 mask, return a 'glow value'
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

//                                       SHADER PROGRAM
// ____________________________________________________________________________________________
//     ( Write all your shader code & functions here )


void main()
{
	vec4 s_layer = texture(tex_layer, TexCoords);
	vec4 s_background = texture(tex_background, TexCoords);

	vec4 final = s_background;
	final = blend_normal(final, vec4(0,0,0,1), kernel_filter_glow(tex_layer, 3, 16, 0));// Drop shadow
	final = blend_normal(final, color_outline, kernel_filter_outline(tex_layer, 3, outline_width) * blend_outline); // outline
	final = blend_normal(final, s_layer, 1.0);

	FragColor = final;
}