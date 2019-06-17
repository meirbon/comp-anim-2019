#version 330

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 tuv;

uniform mat4 view;

out vec2 uv;
out vec2 px;

void main()
{
	uv = tuv;
	px = pos.xy;
	gl_Position = pos;
}
