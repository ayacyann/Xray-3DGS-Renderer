#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in float vo_density[];
in vec2 vo_center[];
in mat2 vo_cov2D[];
in float vo_mu[];

out float go_density;
out vec2 go_center;
out mat2 go_cov2D;
out float go_mu;

uniform vec2 screenSize;

void main()
{
    mat2 cov = vo_cov2D[0];

    // ---- eigen decomposition of 2x2 symmetric matrix ----
    float a = cov[0][0];
    float b = cov[0][1];
    float c = cov[1][1];

    float trace = a + c;
    float det   = a*c - b*b;

    float s = sqrt(max(trace*trace*0.25 - det, 0.0));

    float lambda1 = trace*0.5 + s;
    float lambda2 = trace*0.5 - s;

    // eigenvectors
    vec2 v1;
    if (abs(b) > 1e-6)
        v1 = normalize(vec2(lambda1 - c, b));
    else
        v1 = vec2(1,0);

    vec2 v2 = vec2(-v1.y, v1.x);

    mat2 R = mat2(v1, v2);

    // 3 sigma scale
    vec2 sigma = 3.0 * sqrt(max(vec2(lambda1, lambda2), 0.0));

    // unit square
    vec2 corners[4] = vec2[](
        vec2(-1,-1),
        vec2( 1,-1),
        vec2(-1, 1),
        vec2( 1, 1)
    );

    go_density = vo_density[0];
    go_center  = vo_center[0];
    go_cov2D   = cov;
    go_mu      = vo_mu[0];

    vec4 center = gl_in[0].gl_Position;

    for(int i=0;i<4;i++)
    {
        vec2 p = corners[i];

        // scale -> rotate
        vec2 offset = R * (p * sigma) * 0.05;
        gl_Position = center + vec4(offset, 0.0, 0.0);
        EmitVertex();
    }

    EndPrimitive();
}
