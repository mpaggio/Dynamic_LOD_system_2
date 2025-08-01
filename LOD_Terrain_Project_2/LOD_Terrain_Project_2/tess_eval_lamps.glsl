#version 460 core

layout(isolines, equal_spacing, cw) in;

uniform float spacing = 1.0; // offset verticale tra le curve, se ne hai più di una

vec3 catmullRom(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    return 0.5 * (
        (2.0 * p1) +
        (-p0 + p2) * t +
        (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t * t +
        (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t * t * t
    );
}

void main() {
    float t = gl_TessCoord.x;
    float curveIndex = gl_TessCoord.y; // Intero [0, N) a seconda di gl_TessLevelOuter[0]

    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;
    vec3 p3 = gl_in[3].gl_Position.xyz;

    vec3 pos = catmullRom(p0, p1, p2, p3, t);

    // Offset verticale per curve multiple
    pos.y += spacing * curveIndex;

    gl_Position = vec4(pos, 1.0);
}
