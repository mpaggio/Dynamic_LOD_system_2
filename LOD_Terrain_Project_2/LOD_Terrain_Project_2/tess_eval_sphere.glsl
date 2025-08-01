#version 460 core

layout(triangles, equal_spacing, cw) in; // Patch di triangoli

in vec3 tcCenter[]; //Dal TCS

out vec4 worldPos; //Al Geometry Shader
out vec3 te_normal;

uniform mat4 model;

void main() {
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    // Coordinate baricentriche del vertice generato
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = 1.0 - u - v;

    // Interpolazione baricentrica per patch triangolare
    vec3 pos = w * p0 + u * p1 + v * p2;

    vec3 sphereCenter = tcCenter[0];

    // Sposta i punti sulla superficie sferica (raggio = distanza da centro a vertice)
    float r = length(p0 - sphereCenter);
    vec3 dir = normalize(pos - sphereCenter);
    vec3 sphericalPos = sphereCenter + dir * r;

    te_normal = mat3(model) * dir;
    worldPos = model * vec4(sphericalPos, 1.0);
}

