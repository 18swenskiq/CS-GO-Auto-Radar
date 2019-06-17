#version 330 core
//                                         OPENGL
// ____________________________________________________________________________________________
in vec2 TexCoords;
out vec4 FragColor;

//                                     SAMPLER UNIFORMS
// Image Inputs _______________________________________________________________________________
uniform usampler2D uMainTex;	// Buyzones
uniform sampler2D MainTex;

//                                       SHADER PROGRAM
// ____________________________________________________________________________________________
//     ( Write all your shader code & functions here )

void main(){
	uint flags = texture(uMainTex, TexCoords).r;

	FragColor = vec4(float((flags >> 0) & 0x1U),0,0,1);
}