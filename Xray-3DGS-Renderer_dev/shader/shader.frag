#version 330 core

out vec4 FragColor;

uniform mat4 MVP;
uniform vec3 cameraPos;
uniform float exposure;

in float density;
//in float mark;
in vec3 scale;
in mat3 rotation;
in vec2 center;
in mat3 cov3D;
in mat2 cov2D;
in float mu;

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
    float detCov2D = det2(cov2D);
    if (detCov2D <= 0.0) {
        discard; // Invalid covariance, skip this fragment
    }
    float detInv = 1.0 / detCov2D;
    vec3 cov = vec3(cov2D[0][0], cov2D[0][1], cov2D[1][1]);
    vec3 conic = vec3(cov.z * detInv, -cov.y * detInv, cov.x * detInv);

    vec2 sub = gl_FragCoord.xy - center;
    float power = -0.5 * (conic.x * sub.x * sub.x + conic.z * sub.y * sub.y) - conic.y * sub.x * sub.y;
    if (power > 0){
        discard;
    }
    float factor = density * mu * exp(power);
    if (factor < 1e-6){
        discard;
    }
	FragColor = vec4(vec3(factor * exposure), 1.0);
    FragColor = vec4(1,0,0,1);
}