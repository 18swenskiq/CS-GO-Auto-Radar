#version 330 core
//                                         OPENGL
// ____________________________________________________________________________________________
in vec2 TexCoords;
out vec4 FragColor;

//                                     SAMPLER UNIFORMS
// Image Inputs _______________________________________________________________________________
uniform sampler2D tex_in;	// Background texture
uniform sampler2D tex_in_1;

//                                       SHADER PROGRAM
// ____________________________________________________________________________________________
//     ( Write all your shader code & functions here )
void main()
{
	vec4 sample_a = vec4(texture(tex_in, TexCoords));
	vec4 sample_b = vec4(texture(tex_in_1, TexCoords));
		
	FragColor = vec4(sample_a.g, sample_b.g, 0, sample_a.r);
}