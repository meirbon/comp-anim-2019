#version 330

uniform sampler2D t1;
uniform sampler2D t2;
uniform sampler2D t3;

in vec2 uv;
in vec2 px;
out vec3 pixel;

void main()
{
	// Pixel coords (uv) go from 0, 0 to 1, 1
	// Calculate which texture we need to draw for this pixel
	
	float x = min(1.0, max(uv.x, 0.0));
	float y = min(1.0, max(uv.y, 0.0));
	//float x = (px.x + 1.0) / 2.0;
	//float y = (px.y + 1.0) / 2.0;

	vec2 tc;
	int tex;
	if (x < 0.3333333)
	{
		tex = 1;
		tc = vec2(x * 3.0, y);

	}
	else if (x < 0.666666667)
	{
		tex = 2;
		tc = vec2((x - 0.3333333333) * 3.0, y);
	}
	else
	{
		tex = 3;
		tc = vec2((x - 0.666666667) * 3.0, y);
	}

	float scaleFactor = 0.31;
	float scaleOffset = (1.0 - scaleFactor) * 0.5;

	switch(tex)
	{
	case(1):
		pixel = texture(t1, vec2(tc.x, 1.0 - tc.y)).rgb;
		break;
	case(2):
		pixel = texture(t2, vec2(tc.x, tc.y)).rgb;
		break;
	case(3):
		pixel = texture(t3, vec2(tc.x * scaleFactor + scaleOffset, tc.y)).rgb;
		break;
	default:
		pixel = vec3(1.0, 0.0, 0.0);
		break;
	}
}