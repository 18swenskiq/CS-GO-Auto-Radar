#version 330 core
out vec4 FragColor;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
};

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;

//Lights
uniform DirLight directional;
uniform vec3 color;

//Prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{
	vec3 viewDir = normalize(viewPos - FragPos);

	FragColor = vec4(CalcDirLight(directional, Normal, viewDir) * color, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	//Ambient value
	vec3 ambient = light.ambient;

	vec3 norm = normalize(normal);

	//diffuse
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff;

	return (ambient + diffuse);
}