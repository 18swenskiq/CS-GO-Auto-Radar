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
uniform vec2 bounds_NWU_SS; // North-West coordinate of the playspace (screen space)
uniform vec2 bounds_SEL_SS; // South-East coordinate of the playspace (screen space)

uniform vec2 pos_spawn_ct;	// Location of the CT Spawn	(0-1)
uniform vec2 pos_spawn_t;	// Location of the T Spawn	(0-1)
uniform vec2 bombsite_a;	// Location of bomsite A	(0-1)
uniform vec2 bombsite_b;	// Location of bombsite B	(0-1)

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
	// R: Buzones (0 or 1)
	// G: none
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
//     ( A collection of simple blend modes )

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

// -------------------------------------- sample helpers --------------------------------------

vec4 sample_gradient(float height)
{
	return vec4(texture(tex_gradient, vec2(height, 0)));
}

float get_playspace(vec4 sample_playspace) { return sample_playspace.a; }
float get_playspace_height(vec4 sample_playspace) { return sample_playspace.a * sample_playspace.r; }
float get_playspace_inverse_height(vec4 sample_playspace) { return sample_playspace.a * sample_playspace.g; }

float get_height(vec4 sample_playspace) { return sample_playspace.g; }
float get_baked_light(vec4 sample_playspace) { return sample_playspace.r; }

// -------------------------------------- kernel filters --------------------------------------
// Given an 0-1 mask, return a 'glow value'
float kernel_filter_glow(sampler2D sampler, int channelID = 0, int sample_size = 16)
{
	vec2 pixel_size = 1.0 / vec2(textureSize(sampler, 0));

	float sT = 0;
	int sample_double = sample_size * 2;

	// Process kernel
	for(int x = 0; x <= sample_double; x++){
		for(int y = 0; y <= sample_double; y++){
			sT += texture(sampler, TexCoords + vec2((-sample_size + x) * pixel_size.x, (-sample_size + y) * pixel_size.y))[channelID];
		}
	}

	sT /= (sample_double * sample_double);

	return sT;
}

// Given a 0-1 mask, return an outline drawn around that mask
float kernel_filter_outline(sampler2D sampler, int channelID = 0, int sample_size = 2)
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
vec4 outline_color = vec4(0.8, 0.8, 0.8, 0.6);
vec4 ao_color = vec4(0.0, 0.0, 0.0, 1.0);

vec4 buyzone_color = vec4(0.180, 0.828, 0.225, 0.667);
vec4 objective_color = vec4(0.770, 0.295, 0.171, 1.000);

void main()
{
	vec4 sBackground = vec4(texture(tex_background, TexCoords));
	vec4 sPlayspace = vec4(texture(tex_playspace, TexCoords));
	vec4 sObjectives = vec4(texture(tex_objectives, TexCoords));

	vec4 final = sBackground;
	final = blend_normal(final, ao_color, kernel_filter_glow(tex_playspace, 3, 16));						// Drop shadow
	final = blend_normal(final, sample_gradient(get_playspace_height(sPlayspace)), get_playspace(sPlayspace));	// Playspace
	final = blend_normal(final, outline_color, kernel_filter_outline(tex_playspace, 3, 2));	// Outline

	//final = blend_normal(final, objective_color, sObjectives.r * sObjectives.a);					// Objectives
	//final = blend_normal(final, buyzone_color, sObjectives.g * sObjectives.a);						// Buyzones
	// Return the final output color
	FragColor = final;
}