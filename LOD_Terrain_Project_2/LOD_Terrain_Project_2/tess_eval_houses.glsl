#version 460 core

//UNA ESECUZIONE PER OGNI VERTICE GENERATO DAL TCS

layout(quads, equal_spacing, cw) in;

uniform float SCALE;
uniform mat4 model;
uniform vec3 originalPoints[8 * 6];

uniform sampler2D texture1; //Displacement

in vec3 normal_tcs[];

out vec4 worldPos;
out vec3 normalTES;
out vec2 tesUV;


vec2 remapUV(vec2 uv, vec3 normal) {
    if (abs(normal.z - 1.0) < 0.01) {        // Front (+Z)
        return uv;
    }
    else if (abs(normal.z + 1.0) < 0.01) {   // Back (-Z)
        return vec2(1.0 - uv.x, uv.y);
    }
    else if (abs(normal.x - 1.0) < 0.01) {   // Right (+X)
        return vec2(uv.y, 1.0 - uv.x);
    }
    else if (abs(normal.x + 1.0) < 0.01) {   // Left (-X)
        return vec2(1.0 - uv.y, uv.x);
    }
    else if (abs(normal.y - 1.0) < 0.01) {   // Top (+Y)
        return vec2(uv.x, 1.0 - uv.y);
    }
    else if (abs(normal.y + 1.0) < 0.01) {   // Bottom (-Y)
        return uv;
    }
    return uv; // fallback
}

bool sharesTwoComponents(vec3 pos, vec3 blockVert, float epsilon) {
    int count = 0;
    if (abs(pos.x - blockVert.x) < epsilon) count++;
    if (abs(pos.y - blockVert.y) < epsilon) count++;
    if (abs(pos.z - blockVert.z) < epsilon) count++;
    return count >= 2;
}

vec3 getDisplacedPos(vec2 uv, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
    float u = uv.x;
    float v = uv.y;
    
    vec3 pos = ((1.0 - u) * (1.0 - v) * p0) + (u * (1.0 - v) * p1) + (u * v * p2) + ((1.0 - u) * v * p3);
    
    vec2 mappedUV = remapUV(uv, normal_tcs[0]);
    float height = texture(texture1, mappedUV).r;

    float epsilon = 0.001;
    bool onBlockBorder = false;

    for (int i = 0; i < 8 * 6; ++i) {
        if (sharesTwoComponents(pos, originalPoints[i], epsilon)) {
            onBlockBorder = true;
            break;
        }
    }

    if (onBlockBorder) {
        height = 0.0; // disattiva displacement
    }

    // Sposto lungo la normale
    return pos + normal_tcs[0] * height * SCALE;

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

    // Calcolo normale tramite differenze finite
    vec2 delta = vec2(1.0 / 64.0);  // più piccolo per evitare artefatti

    vec3 posL = getDisplacedPos(uv - vec2(delta.x, 0.0), p0, p1, p2, p3);
    vec3 posR = getDisplacedPos(uv + vec2(delta.x, 0.0), p0, p1, p2, p3);
    vec3 posD = getDisplacedPos(uv - vec2(0.0, delta.y), p0, p1, p2, p3);
    vec3 posU = getDisplacedPos(uv + vec2(0.0, delta.y), p0, p1, p2, p3);

    vec3 dU = posU - posD;
    vec3 dR = posR - posL;

    vec3 normal = normalize(cross(dU, dR));

    if (dot(normal, normal_tcs[0]) < 0.0) {
        normal = -normal;
    }

    worldPos = model * vec4(pos, 1.0);
    normalTES = normalize(transpose(inverse(mat3(model))) * normal);

    tesUV = remapUV(uv, normal_tcs[0]);
}