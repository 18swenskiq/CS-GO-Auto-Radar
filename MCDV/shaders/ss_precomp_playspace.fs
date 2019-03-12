#version 330 core
// Note:: All channels marked with an ** are currently not filled out by the engine.

//                                         OPENGL
// ____________________________________________________________________________________________
in vec2 TexCoords;
out vec4 FragColor;


//                                     SAMPLER UNIFORMS
// Image Inputs _______________________________________________________________________________
uniform sampler2D tex_in;	// Background texture

//                                       SHADER HELPERS
// ____________________________________________________________________________________________
//     ( A collection of simple blend modes )
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
	if(sIn.r > 0.0)
	{
		FragColor = sIn;
	} 
	else
	{
		int sampleCount = 32;
		int outlineWidth = 2;

		float sT = 0;
		float thisHeight = getSample(vec2(0,0)).g;

		for(int x = 0; x <= sampleCount; x++)
		{
			for(int y = 0; y <= sampleCount; y++)
			{
				sT += getSample(vec2(-16 + x,-16 + y)).r;
			}
		}

		sT *= 2;

		float olT = 0;
		for(int x = 0; x <= outlineWidth * 2; x++)
		{
			for(int y = 0; y <= outlineWidth * 2; y++)
			{
				olT += getSample(vec2(-outlineWidth + x,-outlineWidth + y)).r;
			}
		}

		clamp(olT, 0, 1);

		sT /= (sampleCount * sampleCount);
		FragColor = vec4(sIn.r, sIn.g, sT, olT);
	}
}