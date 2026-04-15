#version 330 core

layout (points) in;
layout (points, max_vertices = 1) out;

/*
in VS_OUT{
    float density;
    vec2 center;
    vec4 centerNDC;
    mat2 cov2D;
    float mu;
} gs_in[];
*/

//out float density;
//out vec2 center;
//out mat2 cov2D;
//out float mu;

uniform vec2 screenSize;

void main(){
    //density = gs_in[0].density;
    //center = gs_in[0].center;
    //cov2D = gs_in[0].cov2D;
    //mu = gs_in[0].mu;
    gl_Position = gl_in[0].gl_Position; 
    EmitVertex();
    EndPrimitive();
}