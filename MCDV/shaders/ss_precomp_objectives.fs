#version 330 core
//                                         OPENGL
// ____________________________________________________________________________________________
in vec2 TexCoords;
out vec4 FragColor;

//                                     SAMPLER UNIFORMS
// Image Inputs _______________________________________________________________________________
uniform sampler2D tex_in;	// Buyzones
uniform sampler2D tex_in_1;	// Bombtargets

//                                       SHADER PROGRAM
// ____________________________________________________________________________________________
//     ( Write all your shader code & functions here )

void main()
{
	vec4 sample = vec4(texture(tex_in, TexCoords));
	vec4 sample1 = vec4(texture(tex_in_1, TexCoords));
	vec4 o = sample + sample1;
	FragColor = vec4(o.r, o.g, 0, o.r + o.g);
}