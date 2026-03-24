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

void main()
{
	vec3 dir = normalize(worldPos - cameraPos);
	vec3 normal;
	vec3 intersection = closestEllipsoidIntersection(dir, normal);
	float align = max(0.4, dot(-dir, normal));
	
	if(length(intersection) < 1e-12)
		discard;

	vec4 newPos = MVP * vec4(intersection, 1);
	newPos /= newPos.w;
	FragColor = vec4(vec3(align * density), 1.0);
}