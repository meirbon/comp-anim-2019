#version 410

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 WorldPos0;

struct DirectionalLight
{
    vec3 color;
    vec3 direction;
};

out vec4 FragColor;
uniform sampler2D texture0;

uniform vec3 ambient;
uniform DirectionalLight directionalLight;

void main()
{
    vec3 color = texture(texture0, TexCoord0.xy).rgb;
    vec3 normal = normalize(Normal0);
    float NdL = max(0.15, dot(normal, directionalLight.direction));
    vec3 lighting = color * NdL + ambient * color;

    FragColor = vec4(lighting, 1.0);
}