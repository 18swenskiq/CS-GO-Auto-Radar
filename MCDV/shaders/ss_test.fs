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
	// R: Playable space (0 or 1), 
	// G: Height (0-1 normalized)
	// B: AO map (mask 0-1)
	// A: Outline (mask 0-1) (need to subtract the playable space from this)

uniform sampler2D tex_objectives;	// Objectives
	// R: Buzones (0 or 1)
	// G: Bombsites (0 or 1)
	// **B: Glow map (mask 0-1)
	// **A: Outline (mask 0-1)

uniform sampler2D tex_props;		// Props
	// **R: Props (0 or 1)
	// **G: Height (0-1 normalized)
	// **B: Glow map (mask 0-1)
	// **A: Outline (mask 0-1)

uniform sampler2D tex_gradient;		// Gradient input
	// **RGBA: 256x1 image defining a gradient

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
	return vec4(lerp(a.rgb, b.rgb, b.a * s), b.a + (a.a * s));
}

vec4 blend_add(vec4 a, vec4 b, float s)
{
	return vec4(a.rgb + (b.rgb * s), a.a);
}

vec4 sample_gradient(float height)
{
	return vec4(texture(tex_gradient, vec2(height, 0)));
}

//                                       SHADER PROGRAM
// ____________________________________________________________________________________________
//     ( Write all your shader code & functions here )
vec4 outline_color = vec4(1.0, 1.0, 1.0, 1.0);
vec4 ao_color = vec4(0.0, 0.0, 0.0, 1.0);

void main()
{
	vec4 sBackground = vec4(texture(tex_background, TexCoords));
	vec4 sPlayspace = vec4(texture(tex_playspace, TexCoords));
	vec4 sObjectives = vec4(texture(tex_objectives, TexCoords));

	// Return the final output color
	FragColor = blend_add(blend_normal(blend_normal(blend_normal(sBackground, ao_color, sPlayspace.b), sample_gradient(sPlayspace.g), sPlayspace.r), outline_color, sPlayspace.a -sPlayspace.r), sObjectives, sObjectives.a * 0.75);
}