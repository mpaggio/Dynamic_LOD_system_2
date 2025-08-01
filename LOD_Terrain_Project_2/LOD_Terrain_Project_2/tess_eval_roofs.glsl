#version 460 core

// UNA ESECUZIONE PER OGNI VERTICE GENERATO DAL TCS
layout(quads, equal_spacing, cw) in;

const float SCALE = 0.02;

uniform mat4 model;
uniform vec3 originalPoints[8*6];

uniform sampler2D texture1; // Displacement map

in vec3 normal_tcs[];

out vec4 worldPos;
out vec3 normalTES;
out vec2 tesUV;


// Funzione per rimappare le UV in base alla normale della faccia
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


// Controlla se il punto p è sul segmento AB con tolleranza epsilon
bool isPointOnSegment(vec3 p, vec3 a, vec3 b, float epsilon) {
    vec3 ab = b - a;
    vec3 ap = p - a;

    // Controlla se il vettore ap è parallelo ad ab (cross product vicino a zero)
    if (length(cross(ab, ap)) > epsilon)
        return false;

    // Controlla che p sia all'interno del segmento (prodotto scalare tra 0 e |ab|^2)
    float dotABAP = dot(ab, ap);
    if (dotABAP < 0.0)
        return false;
    float abLenSq = dot(ab, ab);
    if (dotABAP > abLenSq)
        return false;

    return true;
}


// Calcola la posizione displacata, disabilitando il displacement se il punto sta su uno spigolo di uno dei tronchi di piramide
vec3 getDisplacedPos(vec2 uv, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
    float u = uv.x;
    float v = uv.y;

    // Posizione interpolata tramite bilinear interpolation
    vec3 pos = ((1.0 - u) * (1.0 - v) * p0) + (u * (1.0 - v) * p1) + (u * v * p2) + ((1.0 - u) * v * p3);

    vec2 mappedUV = remapUV(uv, normal_tcs[0]);
    float height = texture(texture1, mappedUV).r;

    float epsilon = 0.001;
    bool onBlockBorder = false;

    // Definizione degli spigoli per un tronco di piramide (8 vertici)
    // 4 spigoli base inferiore, 4 spigoli base superiore, 4 spigoli verticali = 12 spigoli * 2 vertici = 24 indici
    int edges[24] = int[](
        0, 1, 1, 2, 2, 3, 3, 0,  // base inferiore
        4, 5, 5, 6, 6, 7, 7, 4,  // base superiore
        0, 4, 1, 5, 2, 6, 3, 7   // spigoli verticali
        );

    // Controlla per entrambi i tronchi di piramide
    for (int trunk = 0; trunk < 6; ++trunk) {
        int baseIndex = trunk * 8;

        for (int i = 0; i < 24; i += 2) {
            vec3 a = originalPoints[baseIndex + edges[i]];
            vec3 b = originalPoints[baseIndex + edges[i + 1]];

            if (isPointOnSegment(pos, a, b, epsilon)) {
                onBlockBorder = true;
                break;
            }
        }
        if (onBlockBorder) break;
    }

    bool isHorizontalFace = abs(normal_tcs[0].y) > 0.99;

    if (onBlockBorder || isHorizontalFace) {
        height = 0.0; // disabilita displacement
    }

    // Sposta la posizione lungo la normale moltiplicata per l'altezza scalata
    return pos + normal_tcs[0] * height * SCALE;
}


void main() {
    // Prende le posizioni 3D di ciascun vertice originale
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;
    vec3 p3 = gl_in[3].gl_Position.xyz;

    // Coordinate baricentriche del vertice generato dalla tessellazione (range [0,1])
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    vec2 uv = vec2(u, v);

    // Calcola la posizione displacata
    vec3 pos = getDisplacedPos(uv, p0, p1, p2, p3);

    // Calcolo della normale tramite differenze finite
    vec2 delta = vec2(1.0 / 64.0);  // passo piccolo per evitare artefatti

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

    bool isHorizontalFace = abs(normal_tcs[0].y) > 0.99;

    worldPos = model * vec4(pos, 1.0);
    normalTES = normalize(transpose(inverse(mat3(model))) * normal);

    if (isHorizontalFace) {
        tesUV = vec2(-1.0, -1.0);
    }
    else {
        tesUV = remapUV(uv, normal_tcs[0]);
    }
}
