#version 330 core
out vec4 FragColor;

// Vert shader ins
in vec2 TexCoords;

// sourcing buffers
uniform sampler2D gPos;
uniform sampler2D gOrigin;
uniform sampler2D refPos;
uniform sampler2D refPos1;

// for reprojection
uniform mat4 projection;
uniform mat4 view;

// Frag main
void main()
{
	// Get the position of the origin in UV cordinates
	vec3 origin = texture(gOrigin, TexCoords).xyz;
	vec4 src = projection * view * vec4(origin, 1.0);
	src.xyz /= src.w;
	src.xyz = src.xyz * 0.5 + 0.5;

	// Sample the reference height buffers and actual height buffer
	float height_reference = texture(refPos, src.xy).r;
	float height_reference1 = texture(refPos1, src.xy).r;
	float height = texture(gPos, TexCoords).g;

	// Initialize output height to 0
	float height_diff = 0;

	// Check whether we should use upper or lower reference buffer.
	if(height < height_reference1){
		// Calculate the relative height from the lower refence buffer
		height_diff = height - height_reference;
	} else {
		// Calculate the relative height from the upper refence buffer
		height_diff = height - height_reference1;
	}

	// Return height as grayscale
	FragColor = vec4(height_diff, 0, 0, 1.0);
}