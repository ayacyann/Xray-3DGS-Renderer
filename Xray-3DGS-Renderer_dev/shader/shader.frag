#version 330 core

out vec4 FragColor;

uniform mat4 MVP;
uniform vec3 cameraPos;

in float density;
in float mark;
in vec3 scale;
in mat3 rotation;
in vec3 worldPos;
in vec3 center;

vec3 closestEllipsoidIntersection(vec3 rayDirection, out vec3 normal) {
  // Convert ray to ellipsoid space
  vec3 localRayOrigin = (cameraPos - center) * rotation;
  vec3 localRayDirection = normalize(rayDirection * rotation);

  vec3 oneover = 1.0 / scale;
  
  // Compute coefficients of quadratic equation
  float a = dot(localRayDirection * oneover, localRayDirection * oneover);
  float b = 2.0 * dot(localRayDirection * oneover, localRayOrigin * oneover);
  float c = dot(localRayOrigin * oneover, localRayOrigin * oneover) - 1.0;
  
  // Compute discriminant
  float discriminant = b * b - 4.0 * a * c;
  
  // If discriminant is negative, there is no intersection
  if (discriminant < 0.0) {
    return vec3(0.0);
  }
  
  // Compute two possible solutions for t
  float t1 = float((-b - sqrt(discriminant)) / (2.0 * a));
  float t2 = float((-b + sqrt(discriminant)) / (2.0 * a));
  
  // Take the smaller positive solution as the closest intersection
  float t = min(t1, t2);
  
  // Compute intersection point in ellipsoid space
  vec3 localIntersection = vec3(localRayOrigin + t * localRayDirection);

  // Compute normal vector in ellipsoid space
  vec3 localNormal = normalize(localIntersection / scale);
  
  // Convert normal vector to world space
  normal = normalize(rotation * localNormal);
  
  // Convert intersection point back to world space
  vec3 intersection = rotation * localIntersection + center;
  
  return intersection;
}

// GLSL 函数：3D高斯投影到2D，高斯协方差使用雅可比矩阵
void projectGaussian3DTo2D(
    out vec2 center2D,      // 投影中心
    out mat2 Sigma2D,   // 投影协方差
    out mat3 Sigma3D
) {
    // --- 1. 构造3D协方差矩阵 ---
    Sigma3D = rotation * mat3(
        scale.x*scale.x, 0.0, 0.0,
        0.0, scale.y*scale.y, 0.0,
        0.0, 0.0, scale.z*scale.z
    ) * transpose(rotation);

    // --- 2. MVP投影 ---
    vec4 ph = MVP * vec4(center, 1.0);
    float w = ph.w;
    center2D = ph.xy / w;

    // --- 3. 计算雅可比矩阵 J ---
    // MVP元素
    float m11 = MVP[0][0], m12 = MVP[0][1], m13 = MVP[0][2], m14 = MVP[0][3];
    float m21 = MVP[1][0], m22 = MVP[1][1], m23 = MVP[1][2], m24 = MVP[1][3];
    float m41 = MVP[3][0], m42 = MVP[3][1], m43 = MVP[3][2], m44 = MVP[3][3];

    float u = center2D.x;
    float v = center2D.y;

    mat2x3 J;
    J[0][0] = (m11 - u*m41)/w;
    J[0][1] = (m12 - u*m42)/w;
    J[0][2] = (m13 - u*m43)/w;

    J[1][0] = (m21 - v*m41)/w;
    J[1][1] = (m22 - v*m42)/w;
    J[1][2] = (m23 - v*m43)/w;

    // --- 4. 投影协方差 Sigma2D = J * Sigma3D * J^T ---
    // 手动展开 mat2 = mat2x3 * mat3x3 * mat3x2
    Sigma2D[0][0] = dot(J[0], Sigma3D * J[0]);
    Sigma2D[0][1] = dot(J[0], Sigma3D * J[1]);
    Sigma2D[1][0] = dot(J[1], Sigma3D * J[0]);
    Sigma2D[1][1] = dot(J[1], Sigma3D * J[1]);
}

float det2(mat2 M) {
    return M[0][0]*M[1][1] - M[0][1]*M[1][0];
}

float det3(mat3 M) {
    return
        M[0][0]*(M[1][1]*M[2][2] - M[1][2]*M[2][1]) -
        M[0][1]*(M[1][0]*M[2][2] - M[1][2]*M[2][0]) +
        M[0][2]*(M[1][0]*M[2][1] - M[1][1]*M[2][0]);
}

void main()
{
    vec2 center2D;
    mat2 Sigma2D;
    mat3 Sigma3D;
    projectGaussian3DTo2D(center2D,Sigma2D,Sigma3D);

	vec3 dir = normalize(worldPos - cameraPos);
	vec3 normal;
	vec3 intersection = closestEllipsoidIntersection(dir, normal);
	
	if(length(intersection) < 1e-6)
		discard;

	vec4 newPos = MVP * vec4(intersection, 1);
	newPos /= newPos.w;
    vec2 d = newPos.xy - center2D;
    float exponent = -0.5 * (d.x * (Sigma2D[0][0]*d.x + Sigma2D[0][1]*d.y) + d.y * (Sigma2D[1][0]*d.x + Sigma2D[1][1]*d.y));
	float align = sqrt(2 * 3.14159265 * det3(Sigma3D) / (det2(Sigma2D) + 1e-12)) * exp(exponent);
	FragColor = vec4(vec3(align * density), 1.0);
}