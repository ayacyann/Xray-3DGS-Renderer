#version 330 core

out vec4 FragColor;

in float Density;
in float Mark;
in vec3 Scale;
in vec4 Rot;

void main()
{
	FragColor = Mark * vec4(1.0, 0.5, 0.2, 1.0);
}