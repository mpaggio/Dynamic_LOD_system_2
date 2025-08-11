#version 460 core

//UNA ESECUZIONE PER OGNI VERTICE GENERATO DAL TCS

layout(quads, equal_spacing, cw) in;

const float SCALE = 0.02;

uniform mat4 model;

uniform sampler2D texture1; //Displacement

in vec4 tcDisplace[];

out vec4 worldPos;
out vec3 normalTES;
out vec2 tesUV;

vec3 getDisplacedPos(vec2 uv, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
    float u = uv.x;
    float v = uv.y;

    vec3 pos = ((1.0 - u) * (1.0 - v) * p0)
        + (u * (1.0 - v) * p1)
        + (u * v * p2)
        + ((1.0 - u) * v * p3);
    
    float height = texture(texture1, uv).r;
    float epsilon = 0.001;

    bool onLeft = abs(uv.x) < epsilon;
    bool onBottom = abs(uv.y) < epsilon;
    bool onRight = abs(uv.x - 1.0) < epsilon;
    bool onTop = abs(uv.y - 1.0) < epsilon;

    if ((onTop && tcDisplace[0].x == 1.0f) || (onRight && tcDisplace[0].y == 1.0f) || (onBottom && tcDisplace[0].z == 1.0f) || (onLeft && tcDisplace[0].w == 1.0f)) {
        height = 0.0f;
    }

    if ((onLeft && onBottom) || (onLeft && onTop) || (onRight && onBottom) || (onRight && onTop)) {
        height = 0.0f;
    }
    
    return pos + vec3(0.0, height * SCALE, 0.0);
}

void main() {
    //Prende le posizioni 3D di ciascun vertice originale
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;
    vec3 p3 = gl_in[3].gl_Position.xyz;

    //Prende le coordinate baricentriche del vertice generato dalla tessellazione (le coordinate del punto normalizzate in [0,1])
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    vec2 uv = vec2(u, v);

    // Calcola posizione displacata
    vec3 pos = getDisplacedPos(uv, p0, p1, p2, p3);

    // Calcolo normale tramite differenze finite simili al terreno montuoso
    vec2 delta = vec2(1.0 / 5.0);

    float hL = texture(texture1, uv - vec2(delta.x, 0.0)).r * SCALE;
    float hR = texture(texture1, uv + vec2(delta.x, 0.0)).r * SCALE;
    float hD = texture(texture1, uv - vec2(0.0, delta.y)).r * SCALE;
    float hU = texture(texture1, uv + vec2(0.0, delta.y)).r * SCALE;

    vec3 dx = vec3(2.0 * delta.x, (hR - hL), 0.0); // moltiplica differenza altezza
    vec3 dz = vec3(0.0, (hU - hD), 2.0 * delta.y);
    vec3 normal = normalize(cross(dz, dx));

    worldPos = model * vec4(pos, 1.0);
    normalTES = normalize(transpose(inverse(mat3(model))) * normal);
    tesUV = uv;
}