#version 460 core

layout(lines) in;
layout(triangle_strip, max_vertices = 128) out;

out vec3 gsFragPos;
out vec3 gsNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

const float radius = 0.01;   // raggio del tubo
const int circleSegments = 12; // segmenti per cerchio
const float extendLength = 0.001; // quanto estendere il tubo ai due lati

// Funzione per calcolare vettore ortogonale arbitrario (non parallelo)
vec3 orthogonal(vec3 v) {
    if (abs(v.x) > abs(v.z))
        return normalize(vec3(-v.y, v.x, 0.0));
    else
        return normalize(vec3(0.0, -v.z, v.y));
}

void main() {
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;

    // Tangente tra i due punti
    vec3 tangent = normalize(p1 - p0);

    // Estendi i punti oltre i bordi del segmento
    vec3 extendedP0 = p0 - tangent * extendLength;
    vec3 extendedP1 = p1 + tangent * extendLength;

    vec3 normal = orthogonal(tangent);
    vec3 binormal = normalize(cross(tangent, normal));

    for (int i = 0; i <= circleSegments; i++) {
        float angle = float(i) / float(circleSegments) * 2.0 * 3.14159265359;
        float cosA = cos(angle);
        float sinA = sin(angle);

        vec3 offset = normal * cosA * radius + binormal * sinA * radius;

        vec3 pos0 = extendedP0 + offset;
        vec3 pos1 = extendedP1 + offset;

        gsFragPos = pos0;
        gsNormal = normalize(offset);
        gl_Position = proj * view * model * vec4(pos0, 1.0);
        EmitVertex();

        gsFragPos = pos1;
        gsNormal = normalize(offset);
        gl_Position = proj * view * model * vec4(pos1, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}
