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

uniform vec4 outline_color;// = vec4(0.8, 0.8, 0.8, 0.6);
uniform vec4 ao_color;// = vec4(0.0, 0.0, 0.0, 1.0);

uniform vec4 buyzone_color;// = vec4(0.180, 0.828, 0.225, 0.667);
uniform vec4 objective_color;// = vec4(0.770, 0.295, 0.171, 1.000);
uniform vec4 cover_color;// = vec4(0.700, 0.700, 0.700, 1.000);

//                                     SAMPLER UNIFORMS
// Image Inputs _______________________________________________________________________________
//    ( Standard generated maps from the engine )

uniform usampler2D umask_playspace;
uniform usampler2D umask_objectives;
uniform usampler2D umask_buyzones;

uniform sampler2D gbuffer_position;
uniform sampler2D gbuffer_normals;

uniform sampler2D tex_background;
uniform sampler2D tex_gradient;		// Gradient input
	// RGBA: 256x1 image defining a gradient

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


// -------------------------------------- sample helpers --------------------------------------

vec4 sample_gradient(float height)
{
	return vec4(texture(tex_gradient, vec2(remap(height, bounds_SEL.y, bounds_NWU.y, 0, 1), 0)));
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
	vec4 sBackground = vec4(texture(tex_background, TexCoords));
	vec4 sGBPosition = vec4(texture(gbuffer_position, TexCoords));
	uint mPlayspace = texture(umask_playspace, TexCoords).r;

	vec4 final = sBackground;

	final = blend_normal(final, sample_gradient(sGBPosition.b), float(mPlayspace));	// Playspace

	// Return the final output color
	FragColor = texture(gbuffer_position, TexCoords);
}