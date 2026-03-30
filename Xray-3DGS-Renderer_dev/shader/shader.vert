#version 330 core

layout (location = 0) in vec3 aCubePos;
layout (location = 1) in vec3 aCenter;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aDensity;
//layout (location = 4) in float aMark;
layout (location = 5) in vec3 aScale;
layout (location = 6) in vec4 aRot;

out float density;
//out float mark;
out vec3 scale;
out mat3 rotation;
out vec3 worldPos;
out vec3 center;
out mat3 cov3D;
out mat2 cov2D;
out float mu;

uniform mat4 MVP;
uniform mat4 viewMatrix;
uniform float fov;

mat3 computeCov3D(vec3 scale, mat3 rot){
    mat3 scaleMat = mat3(
        vec3(scale.x, 0.0, 0.0),
        vec3(0.0, scale.y, 0.0),
        vec3(0.0, 0.0, scale.z)
    );
    mat3 M = scaleMat * rot;
    return transpose(M) * M;
}

mat2 computeCov2D(vec3 worldCenterPos, float fov, mat3 cov3D, out float mu){
    vec3 centerInView = vec3(viewMatrix * vec4(worldCenterPos, 1.0));
    float tanFov = tan(radians(fov) / 2.0);
    float limx = 1.3f * tanFov;
    float limy = 1.3f * tanFov;
    float txtz = centerInView.x / centerInView.z;
    float tytz = centerInView.y / centerInView.z;
    centerInView.x = min(limx, max(-limx, txtz)) * centerInView.z;
    centerInView.y = min(limy, max(-limy, tytz)) * centerInView.z;
    float l = length(centerInView);
    mat3 J = mat3(
        vec3(fov / centerInView.z, 0.0, -(fov * centerInView.x / (centerInView.z * centerInView.z))),
        vec3(0.0, fov / centerInView.z, -(fov * centerInView.y / (centerInView.z * centerInView.z))),
        vec3(centerInView.x / l, centerInView.y / l, centerInView.z / l)
    );
    mat3 W = mat3(
        vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]),
        vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]),
        vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2])
    );
    mat3 M = W * J;
    mat3 cov = transpose(M) * cov3D * M;
    float hata = cov[0][0];
    float hatb = cov[0][1];
    float hatc = cov[0][2];
    float hatd = cov[1][1];
    float hate = cov[1][2];
    float hatf = cov[2][2];
    float diamond = hata * hatd - hatb * hatb;
    float circ = hata * hatd * hatf + 2.0 * hatb * hatc * hate - hata * hate * hate - hatd * hatc * hatc - hatf * hatb * hatb;
    float muSquare = 2 * 3.1415926 * circ / diamond;
    if (muSquare > 0.0) {
        mu = sqrt(muSquare);
    }
    mat2 cov2D = mat2(
        vec2(hata, hatb),
        vec2(hatb, hatd)
    );
    return cov2D;
}

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
    //mark = aMark;
    density = softplus(aDensity);
    scale = exp(aScale).xzy;
    rotation = transpose(quatToMat3(aRot));
    center = aCenter.xzy;
    cov3D = computeCov3D(scale, rotation);
    mu = 0.0;
    cov2D = computeCov2D(center, fov, cov3D, mu);
    worldPos = rotation * scale * aCubePos + center;
    gl_Position = MVP * vec4(worldPos, 1.0);
}