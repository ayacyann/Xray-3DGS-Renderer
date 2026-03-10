#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in float aDensity;
layout (location = 3) in float aMark;
layout (location = 4) in vec3 aScale;
layout (location = 5) in vec4 aRot;

out float Density;
out float Mark;
out vec3 Scale;
out vec4 Rot;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Mark = aMark;

    gl_Position = projection * view * model * vec4(aPos, 1.0f);

}