#version 330 core
//                                         OPENGL
// ____________________________________________________________________________________________
in vec2 TexCoords;
out vec4 FragColor;

//                                     SAMPLER UNIFORMS
// Image Inputs _______________________________________________________________________________
uniform sampler2D tex_layer;
uniform sampler2D gbuffer_position;

uniform float saturation;
uniform float value;

//                                       SHADER HELPERS
// ____________________________________________________________________________________________
// --------------------------------------- Blend modes ----------------------------------------

float lerp(float a, float b, float w)
{
	return a + w*(b-a);
}

vec3 lerp(vec3 a, vec3 b, float w)
{
  return a + w*(b-a);
}

vec4 lerp(vec4 a, vec4 b, float w)
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

//https://gamedev.stackexchange.com/a/59808
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

//                                       SHADER PROGRAM
// ____________________________________________________________________________________________
//     ( Write all your shader code & functions here )

void main()
{
	vec4 s_layer = texture(tex_layer, TexCoords);
	vec4 s_gbuffer = texture(gbuffer_position, TexCoords);
	
	vec3 colHSV = rgb2hsv(s_layer.rgb);
	colHSV.g *= saturation;
	colHSV.b *= value;
	vec3 colRGB = hsv2rgb(colHSV);

	FragColor = vec4(colRGB, s_layer.a);
}