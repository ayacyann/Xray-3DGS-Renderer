#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in float vo_density[];
in vec2 vo_center[];
in mat2 vo_cov2D[];
in float vo_mu[];
in float vo_radius[];

out float go_density;
out vec2 go_center;
out mat2 go_cov2D;
out float go_mu;

uniform vec2 screenSize;

float det2(mat2 M) {
    return M[0][0]*M[1][1] - M[0][1]*M[1][0];
}

void main()
{
    // Discard if vertex shader signaled invalid Gaussian (radius < 0)
    if (vo_radius[0] < 0.0)
        return;

    vec4 center_clip = gl_in[0].gl_Position;
    float radius = vo_radius[0];

    // Convert pixel offset to clip-space offset
    // 1 pixel in NDC = 2.0 / screen_size
    // Clip offset = NDC offset * w
    float dx_clip = radius * 2.0 / screenSize.x * center_clip.w;
    float dy_clip = radius * 2.0 / screenSize.y * center_clip.w;

    vec4 offsets[4] = vec4[4](
        vec4(-dx_clip, -dy_clip, 0.0, 0.0),  // bottom-left
        vec4( dx_clip, -dy_clip, 0.0, 0.0),  // bottom-right
        vec4(-dx_clip,  dy_clip, 0.0, 0.0),  // top-left
        vec4( dx_clip,  dy_clip, 0.0, 0.0)   // top-right
    );

    // Pass through uniform values (same for all quad corners)
    go_cov2D      = vo_cov2D[0];
    go_density    = vo_density[0];
    go_mu         = vo_mu[0];
    go_center = vo_center[0];

    for (int i = 0; i < 4; i++) {
        gl_Position = center_clip + offsets[i];
        EmitVertex();
    }
    EndPrimitive();
}
