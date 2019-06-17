#version 410

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 WorldPos0;

uniform mat4 MVP;
uniform mat4 gWorld;

void main()
{
	vec4 PosL = vec4(Position, 1.0);
	gl_Position = MVP * PosL;
	TexCoord0 = TexCoord;

	vec4 NormalL = vec4(Normal, 0.0);
	Normal0 = (gWorld * vec4(NormalL.xyz, 0.0)).xyz;
	WorldPos0 = (gWorld * PosL).xyz;
}
