#version 330 core

out vec4 FragColor;

uniform mat4 MVP;
uniform vec3 cameraPos;
uniform float exposure;

in float density;
//in float mark;
in vec3 scale;
in mat3 rotation;
in vec3 worldPos;
in vec3 center;
in mat3 cov3D;
in mat2 cov2D;
in float mu;

vec3 closestEllipsoidIntersection(vec3 rayDirection) {
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
  
  // Convert intersection point back to world space
  vec3 intersection = rotation * localIntersection + center;
  
  return intersection;
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
	vec3 dir = normalize(worldPos - cameraPos);
	vec3 intersection = closestEllipsoidIntersection(dir);
	
	if(length(intersection) < 1e-6)
		discard;

	vec4 newPos = MVP * vec4(intersection, 1);
	newPos /= newPos.w;
	FragColor = vec4(vec3(density * exposure), 1.0);
}