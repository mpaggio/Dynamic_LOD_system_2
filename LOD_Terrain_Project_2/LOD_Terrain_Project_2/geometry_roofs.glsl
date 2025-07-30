#version 460 core

//UNA ESECUZIONE PER OGNI PRIMITIVA GENERATA DAL TES


layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec4 worldPos[];
in vec3 normalTES[];
in vec2 tesUV[];

out vec2 tesUV_gs;
out vec3 worldPos_gs;
out vec3 normal_gs;

uniform mat4 view;
uniform mat4 proj;

void main() {
    for (int i = 0; i < 3; ++i) {
        gl_Position = proj * view * worldPos[i];
        tesUV_gs = tesUV[i];
        normal_gs = normalTES[i];
        worldPos_gs = worldPos[i].xyz;
        EmitVertex();
    }
    EndPrimitive();
}
