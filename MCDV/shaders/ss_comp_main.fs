#version 330 core
// Note:: All channels marked with an ** are currently not filled out by the engine.

//                                         OPENGL
// ____________________________________________________________________________________________
in vec2 TexCoords;
out vec4 FragColor;

//                                        UNIFORMS
// Vector Information _________________________________________________________________________
//    ( A bunch of vectors that give you the location of different entities )
uniform vec3 bounds_NWU;	// North-West-Upper coordinate of the playspace (worldspace)
uniform vec3 bounds_SEL;	// South-East-Lower coordinate of the playspace (worldspace)
uniform vec2 bounds_NWU_SS; // **North-West coordinate of the playspace (UV Screenspace)
uniform vec2 bounds_SEL_SS; // **South-East coordinate of the playspace (UV Screenspace)

uniform vec2 pos_spawn_ct;	// **Location of the CT Spawn	(UV Screenspace)
uniform vec2 pos_spawn_t;	// **Location of the T Spawn	(UV Screenspace)
uniform vec2 bombsite_a;	// **Location of bomsite A	    (UV Screenspace)
uniform vec2 bombsite_b;	// **Location of bombsite B	    (UV Screenspace)

uniform int cmdl_shadows_enable;	// Commandline switch --ao

uniform int cmdl_ao_enable;			// Commandline switch --shadows
uniform int cmdl_ao_size;

uniform int cmdl_outline_enable;
uniform int cmdl_outline_size;

//                                     SAMPLER UNIFORMS
// Image Inputs _______________________________________________________________________________
//    ( Standard generated maps from the engine )
uniform sampler2D tex_background;	// Background texture
uniform sampler2D tex_playspace;	// Playspace 
	// R: Height (Regular)
	// G: Height (Reverse rendering order)
	// **B: Baked Lighting
	// A: Playable Space (0 or 1)

uniform sampler2D tex_objectives;	// Objectives
	// R: Objectives (0 or 1)
	// G: Buzones (0 or 1)
	// B: none
	// A: Buyzones & Bombsites (mask 0-1)

uniform sampler2D tex_props;		// Props
	// **R: Height (0-1 normalized)
	// **G: none
	// **B: none
	// **A: Props (0 or 1)

uniform sampler2D tex_gradient;		// Gradient input
	// RGBA: 256x1 image defining a gradient

uniform sampler2D texture0; // Custom Image input 3 (**RGBA)
uniform sampler2D texture1; // Custom Image input 4 (**RGBA)
uniform sampler2D texture2; // Custom Image input 5 (**RGBA)

//                                       SHADER HELPERS
// ____________________________________________________________________________________________
// --------------------------------------- Blend modes ----------------------------------------

vec3 lerp(vec3 a, vec3 b, float w)
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

// ---------------------------------------- transforms ----------------------------------------
vec2 worlspaceToScreen(vec3 worldCoord)
{
	return vec2(
		remap(worldCoord.x, bounds_NWU.x, bounds_SEL.x, 0, 1), 
		remap(worldCoord.y, bounds_SEL.y, bounds_NWU.y, 0, 1));
}

vec3 screenSpaceToWorld(vec2 screenCoord, float heightSample)
{
	return vec3(
		remap(screenCoord.x, 0, 1, bounds_NWU.x, bounds_SEL.x),
		remap(screenCoord.y, 0, 1, bounds_SEL.y, bounds_NWU.y),
		remap(heightSample, 0, 1, bounds_SEL.z, bounds_NWU.z));
}

float screenHeightToWorld(float screenHeight)
{
	return remap(screenHeight, 0, 1, bounds_SEL.z, bounds_NWU.z);
}

// -------------------------------------- sample helpers --------------------------------------

vec4 sample_gradient(float height)
{
	return vec4(texture(tex_gradient, vec2(height, 0)));
}

float get_playspace(vec4 sample_playspace) { return sample_playspace.a; }
float get_playspace_height(vec4 sample_playspace) { return sample_playspace.a * sample_playspace.r; }
float get_playspace_inverse_height(vec4 sample_playspace) { return sample_playspace.a * sample_playspace.g; }

float get_height(vec4 sample_playspace) { return sample_playspace.r; }


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

// Given a height map, return a shadow map for this sample
float trace_shadow(sampler2D heightSampler, int channelID)
{
	int traceCount = 1024;

	vec3 angleV = vec3(-0.5, 1, 1);

	vec3 sampleWorldCoord = screenSpaceToWorld(TexCoords, texture(heightSampler, TexCoords).r);

	float sD = 0;

	for(int i = 0; i < traceCount; i++)
	{
		vec3 traceOffset = angleV * i;
		vec3 stepSampleLoc = sampleWorldCoord + traceOffset;
		vec2 uvSampleLoc = worlspaceToScreen(stepSampleLoc);
		float stepSampleHeight = screenHeightToWorld(texture(heightSampler, uvSampleLoc).r);

		if(stepSampleHeight-sampleWorldCoord.z > traceOffset.z)
		{
			sD += 1;
		}
	}

	return clamp(sD, 0, 1);
}

// Given a height map, return an ambient occlusion term for this sample
// This is a wip, expect kinda shitty results :)
float kernel_ao_basic(sampler2D sampler, int channelID, int sample_size)
{
	vec2 pixel_size = 1.0 / vec2(textureSize(sampler, 0));

	float sT = 0;
	int sample_double = sample_size * 2;

	float thisHeight = texture(sampler, TexCoords)[channelID];

	// Process kernel
	for(int x = 0; x <= sample_double; x++){
		for(int y = 0; y <= sample_double; y++){
			float dif = texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y))[channelID] - thisHeight;
			sT += clamp(dif, 0, 0.04);
		}
	}

	sT /= (sample_double * sample_double);
	sT *= 16;

	return sT;
}

//                                       SHADER PROGRAM
// ____________________________________________________________________________________________
//     ( Write all your shader code & functions here )
vec4 outline_color = vec4(0.8, 0.8, 0.8, 0.6);
vec4 ao_color = vec4(0.0, 0.0, 0.0, 1.0);

vec4 buyzone_color = vec4(0.180, 0.828, 0.225, 0.667);
vec4 objective_color = vec4(0.770, 0.295, 0.171, 1.000);
vec4 cover_color = vec4(0.700, 0.700, 0.700, 1.000);

void main()
{
	vec4 sBackground = vec4(texture(tex_background, TexCoords));
	vec4 sPlayspace = vec4(texture(tex_playspace, TexCoords));
	vec4 sObjectives = vec4(texture(tex_objectives, TexCoords));

	vec4 final = sBackground;
	final = blend_normal(final, ao_color, kernel_filter_glow(tex_playspace, 3, 16, 0));							// Drop shadow
	final = blend_normal(final, sample_gradient(get_playspace_height(sPlayspace)), get_playspace(sPlayspace));	// Playspace
	final = blend_normal(final, cover_color, sPlayspace.b);														// Cover

	if(cmdl_shadows_enable == 1) final = blend_normal(final, vec4(0,0,0,1), trace_shadow(tex_playspace, 0) * 0.2);		// Shadows
	if(cmdl_ao_enable == 1) final = blend_normal(final, vec4(0,0,0,1), kernel_ao_basic(tex_playspace, 0, cmdl_ao_size) * 0.9);	// AO

	if(cmdl_outline_enable == 1) final = blend_normal(final, outline_color, kernel_filter_outline(tex_playspace, 3, cmdl_outline_size));						// Outline

	final = blend_normal(final, objective_color,																// Objectives
		(kernel_filter_glow(tex_objectives, 0, 16, 1) * sObjectives.r) + 
		kernel_filter_outline(tex_objectives, 0, 2));

	final = blend_normal(final, buyzone_color,																	// Buyzones
		(kernel_filter_glow(tex_objectives, 1, 16, 1) * sObjectives.g) + 
		kernel_filter_outline(tex_objectives, 1, 2));
	

	// Return the final output color
	FragColor = final;
}