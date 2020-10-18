#version 330 core
layout (location = 0) in vec3 va_Pos;
layout (location = 1) in vec3 va_Normal;
layout (location = 2) in vec2 va_Origin;

out vec3 sh_FragPos;
out vec3 sh_Normal;
out vec2	sh_Origin;

uniform mat4 in_Projection;
uniform mat4 in_Transform;

void main()
{
	vec4 wpos = vec4( va_Pos, 1.0 );

	sh_FragPos = vec3( in_Transform * wpos );

	mat3 normalMatrix = transpose( inverse( mat3( in_Transform )));
   sh_Normal = normalMatrix * va_Normal;

	sh_Origin = vec2( in_Transform * vec4(va_Origin, 0.0, 1.0) );

	gl_Position = in_Projection * in_Transform * wpos;
}
