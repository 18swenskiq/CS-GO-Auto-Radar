#version 330 core
//                                         OPENGL
// ____________________________________________________________________________________________
in vec2 TexCoords;
out vec4 FragColor;

//                                     SAMPLER UNIFORMS
// Image Inputs _______________________________________________________________________________
uniform sampler2D tex_in;	// Background texture

//                                       SHADER HELPERS
// ____________________________________________________________________________________________
//     ( Simple sample with offset )
vec2 pixel_size = 1.0 / vec2(textureSize(tex_in, 0));
vec4 getSample(vec2 offset)
{
	return vec4(texture(tex_in, TexCoords + (offset * pixel_size)));
}

//                                       SHADER PROGRAM
// ____________________________________________________________________________________________
//     ( Write all your shader code & functions here )

void main()
{
	vec4 sIn = getSample(vec2(0,0));

	//Temp
	int sampleCount = 64;
	int outlineWidth = 1;

	vec2 sT = vec2(0, 0);

	// Glow sampler ==========================================================
	for(int x = 0; x <= sampleCount; x++)
	{
		for(int y = 0; y <= sampleCount; y++)
		{
			sT += (vec2(1, 1)-getSample(vec2(-32 + x,-32 + y)).rg) * (1 - ((abs(-32 +x) * abs(-32 +y)) / 512));
		}
	}

	// Outline sampler =======================================================
	vec2 olT = vec2(0, 0);
	for(int x = 0; x <= outlineWidth * 2; x++)
	{
		for(int y = 0; y <= outlineWidth * 2; y++)
		{
			olT += vec2(1, 1) - getSample(vec2(-outlineWidth + x,-outlineWidth + y)).rg;
		}
	}

	float global_opacity = 0.075;

	sT /= (sampleCount * sampleCount);
	sT = vec2(pow(sT.r, 1.5), pow(sT.g, 1.5));
	sT *= 0.75;
	FragColor = vec4((sIn.r * olT.r) + (sIn.r * sT.r) + (sIn.r * global_opacity), (sIn.g * olT.g) + (sIn.g * sT.g) + (sIn.g * global_opacity), 0, sIn.r + sIn.g);
}