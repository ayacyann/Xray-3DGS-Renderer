#version 330 core

layout (location = 0) in vec3 aCubePos;
layout (location = 1) in vec3 aCenter;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aDensity;
layout (location = 4) in float aMark;
layout (location = 5) in vec3 aScale;
layout (location = 6) in vec4 aRot;

out float density;
out float mark;
out vec3 scale;
out mat3 rotation;
out vec3 worldPos;
out vec3 center;

uniform mat4 MVP;

mat3 quatToMat3(vec4 q) {
  float qx = q.y;
  float qy = q.z;
  float qz = q.w;
  float qw = q.x;

  float qxx = qx * qx;
  float qyy = qy * qy;
  float qzz = qz * qz;
  float qxz = qx * qz;
  float qxy = qx * qy;
  float qyw = qy * qw;
  float qzw = qz * qw;
  float qyz = qy * qz;
  float qxw = qx * qw;

  return mat3(
    vec3(1.0 - 2.0 * (qyy + qzz), 2.0 * (qxy - qzw), 2.0 * (qxz + qyw)),
    vec3(2.0 * (qxy + qzw), 1.0 - 2.0 * (qxx + qzz), 2.0 * (qyz - qxw)),
    vec3(2.0 * (qxz - qyw), 2.0 * (qyz + qxw), 1.0 - 2.0 * (qxx + qyy))
  );
}

float softplus(float x) {
    return log(1.0 + exp(x));
}

void main()
{
    mark = aMark;
    density = softplus(aDensity);
    scale = exp(aScale);
    rotation = transpose(quatToMat3(aRot));
    center = aCenter.xzy;
    worldPos = rotation * scale * aCubePos + center;
    gl_Position = MVP * vec4(worldPos, 1.0);
}