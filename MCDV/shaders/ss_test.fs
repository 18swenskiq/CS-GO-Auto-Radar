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
	// **B: AO map (mask 0-1)
	// **A: Outline (mask 0-1))

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

//                                       SHADER PROGRAM
// ____________________________________________________________________________________________
//     ( Write all your shader code & functions here )

void main()
{
	vec4 sBackground = vec4(texture(tex_background, TexCoords));
	vec4 sObjectives = vec4(texture(tex_objectives, TexCoords));

	// Return the final output color
	FragColor = vec4(sBackground.rgb, 1);//-sBackground;
}