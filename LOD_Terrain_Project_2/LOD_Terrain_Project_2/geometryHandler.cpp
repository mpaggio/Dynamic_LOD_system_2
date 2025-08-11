#include "geometryHandler.h"


// --- SKYBOX --- //
vector<float> generateSkyboxCube() {
    vector<float> skyboxVertices = vector<float>{
        -1.0f,  1.0f, -1.0f,  // fronte
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,  // retro
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,  // destra
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,  // sinistra
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,  // alto
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,  // basso
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    return skyboxVertices;
}




// --- PLANE --- //
vector<float> simplePlane(int division, float width) {
	vector<float> plane;
	float triangleSide = width / division;

	for (int row = 0; row < division + 1; row++) {
		for (int col = 0; col < division + 1; col++) {
			vec3 vertex = vec3(col * triangleSide, 0.0, row * -triangleSide);
			plane.push_back(vertex.x);
			plane.push_back(vertex.z);
			plane.push_back(vertex.y);
		}
	}
	return plane;
}




// --- ROAD AND GRASS --- //
pair<vector<float>, vector<bool>> roadAndGrass(int division, float width, int roadWidth) {
    vector<float> vertices;
    vector<bool> isRoad;

    float cellSize = width / division;
    int center = division / 2; // indice centrale

    for (int row = 0; row <= division; row++) {
        for (int col = 0; col <= division; col++) {
            vec3 vertex = vec3(col * cellSize, 0.0f, row * -cellSize);

            // Definisco la croce centrale con larghezza roadWidth
            bool isRoadCell = (col >= center - roadWidth / 2 && col <= center + roadWidth / 2) ||
                (row >= center - roadWidth / 2 && row <= center + roadWidth / 2);

            vertices.push_back(vertex.x);
            vertices.push_back(vertex.z);
            vertices.push_back(vertex.y);

            isRoad.push_back(isRoadCell);
        }
    }

    return { vertices, isRoad };
}






// --- PLANE PATCHES --- //
tuple<vector<float>, vector<vec4>, vector<float>, vector<vec4>> generatePatches(const vector<float>& plane, const vector<bool>& isRoad, int division) {
    vector<float> roadPatches;
    vector<vec4> roadEdges; // x(top), y(right), z(bottom), w(left)
    vector<float> grassPatches; 
    vector<vec4> grassEdges; // x(top), y(right), z(bottom), w(left)

    int rowLength = division + 1;

    auto getPatchFlag = [&](int row, int col) -> vec4 {
        bool top = false, right = false, bottom = false, left = false;
        bool selfRoad = isRoad[row * (division + 1) + col];
        bool targetNeighbor = !selfRoad;

        // Controlla sopra (top)
        if (row > 0 && isRoad[(row - 1) * (division + 1) + col] == targetNeighbor)
            top = true;
        // Controlla destra (right)
        if (col < division - 1 && isRoad[row * (division + 1) + (col + 1)] == targetNeighbor)
            right = true;
        // Controlla sotto (bottom)
        if (row < division - 1 && isRoad[(row + 1) * (division + 1) + col] == targetNeighbor)
            bottom = true;
        // Controlla sinistra (left)
        if (col > 0 && isRoad[row * (division + 1) + (col - 1)] == targetNeighbor)
            left = true;
        
        return vec4(
            top ? 1.0f : 0.0f,
            right ? 1.0f : 0.0f,
            bottom ? 1.0f : 0.0f,
            left ? 1.0f : 0.0f
        );
    };

    for (int row = 0; row < division; ++row) {
        for (int col = 0; col < division; ++col) {
            
            int idx[4] = {
                ((row + 1) * rowLength + col) * 3,       // bottom-left
                ((row + 1) * rowLength + col + 1) * 3,   // bottom-right
                (row * rowLength + col + 1) * 3,         // top-right
                (row * rowLength + col) * 3              // top-left
            };

            bool patchIsRoad = isRoad[row * (division + 1) + col];
            vec4 flag = getPatchFlag(row, col);

            vector<vec4>& targetEdges = patchIsRoad ? roadEdges : grassEdges;
            vector<float>& targetPatches = patchIsRoad ? roadPatches : grassPatches;

            for (int i = 0; i < 4; ++i) {
                targetEdges.push_back(flag);
                targetPatches.push_back(plane[idx[i]]);
                targetPatches.push_back(plane[idx[i] + 1]);
                targetPatches.push_back(plane[idx[i] + 2]);
            }
        }
    }
    return { roadPatches, roadEdges, grassPatches, grassEdges};
}



// SPHERES
pair<vector<vec3>, vector<vec3>> generateSphericalBasesFromPositions(const vector<vec3>& basePositions) {
    vector<vec3> verts;
    vector<vec3> centers;

    // Generatore random e distribuzione per il raggio unico
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<float> radiusDist(0.06f, 0.1f);

    float radius = radiusDist(gen);  // scelgo un raggio casuale UNA VOLTA

    // Definizione fissa degli angoli/sfera
    vector<vec3> sphereCorners = {
        vec3(0,  1,  0),
        vec3(0,  -1,  0),
        vec3(0, 0,  1),
        vec3(0, 0,  -1),
        vec3(1,  0, 0),
        vec3(-1,  0, 0)
    };

    const int ottanteTriangles[8][3] = {
        {0, 2, 4}, // Ottante 1
        {0, 3, 4}, // Ottante 2
        {0, 3, 5}, // Ottante 3
        {0, 2, 5}, // Ottante 4
        {1, 2, 4}, // Ottante 5
        {1, 3, 4}, // Ottante 6
        {1, 3, 5}, // Ottante 7
        {1, 2, 5}  // Ottante 8
    };

    for (const vec3& topVertex : basePositions) {
        vec3 center = topVertex - vec3(0, radius, 0);  // Calcolo centro

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 3; ++j) {
                vec3 dir = normalize(sphereCorners[ottanteTriangles[i][j]]);
                vec3 offset = dir * radius;
                verts.push_back(center + offset);
                centers.push_back(center);
            }
        }
    }

    return { verts, centers };
}


tuple<vector<float>, vector<float>, vector<vec3>> generateBlocks(const vector<vec3>& positions, int subdivisions, bool isHedge) {
    float baseSize = 1.0f;
    float minHeight = isHedge ? 0.4f : 1.0f;
    float maxHeight = isHedge ? 0.6f : 2.5f;

    vector<float> blocks;
    vector<float> heights;
    vector<vec3> baseVertices;

    blocks.reserve(positions.size() * subdivisions * 8 * 3);
    heights.reserve(positions.size());
    baseVertices.reserve(positions.size() * 8);

    srand(static_cast<unsigned int>(time(nullptr)));

    for (const vec3& baseCenter : positions) {
        float totalHeight = minHeight + static_cast<float>(rand()) / RAND_MAX * (maxHeight - minHeight);
        heights.push_back(totalHeight);

        float segmentHeight = totalHeight / subdivisions;

        float halfWidth = isHedge
            ? (0.2f + static_cast<float>(rand()) / RAND_MAX * 0.8f) / 2.0f  // max 1.0
            : baseSize / 2.0f;

        float halfDepth = isHedge
            ? (0.2f + static_cast<float>(rand()) / RAND_MAX * 0.8f) / 2.0f  // max 1.0
            : baseSize / 2.0f;

        // Calcolo vertici della base originale
        vec3 p0 = baseCenter + vec3(-halfWidth, 0.0f, -halfDepth);
        vec3 p1 = baseCenter + vec3(halfWidth, 0.0f, -halfDepth);
        vec3 p2 = baseCenter + vec3(halfWidth, 0.0f, halfDepth);
        vec3 p3 = baseCenter + vec3(-halfWidth, 0.0f, halfDepth);

        vec3 p4 = p0 + vec3(0.0f, totalHeight, 0.0f);
        vec3 p5 = p1 + vec3(0.0f, totalHeight, 0.0f);
        vec3 p6 = p2 + vec3(0.0f, totalHeight, 0.0f);
        vec3 p7 = p3 + vec3(0.0f, totalHeight, 0.0f);

        baseVertices.push_back(p0);
        baseVertices.push_back(p1);
        baseVertices.push_back(p2);
        baseVertices.push_back(p3);
        baseVertices.push_back(p4);
        baseVertices.push_back(p5);
        baseVertices.push_back(p6);
        baseVertices.push_back(p7);

        // Suddivisioni verticali
        for (int i = 0; i < subdivisions; ++i) {
            float y0 = i * segmentHeight;
            float y1 = y0 + segmentHeight;

            vec3 segmentBaseCenter = baseCenter + vec3(0.0f, y0, 0.0f);

            vec3 sp0 = segmentBaseCenter + vec3(-halfWidth, 0.0f, -halfDepth);
            vec3 sp1 = segmentBaseCenter + vec3(halfWidth, 0.0f, -halfDepth);
            vec3 sp2 = segmentBaseCenter + vec3(halfWidth, 0.0f, halfDepth);
            vec3 sp3 = segmentBaseCenter + vec3(-halfWidth, 0.0f, halfDepth);

            vec3 sp4 = sp0 + vec3(0.0f, segmentHeight, 0.0f);
            vec3 sp5 = sp1 + vec3(0.0f, segmentHeight, 0.0f);
            vec3 sp6 = sp2 + vec3(0.0f, segmentHeight, 0.0f);
            vec3 sp7 = sp3 + vec3(0.0f, segmentHeight, 0.0f);

            vec3 segmentCube[8] = { sp0, sp1, sp2, sp3, sp4, sp5, sp6, sp7 };

            for (int v = 0; v < 8; ++v) {
                blocks.push_back(segmentCube[v].x);
                blocks.push_back(segmentCube[v].y);
                blocks.push_back(segmentCube[v].z);
            }
        }
    }

    return { blocks, heights, baseVertices };
}

pair<vector<float>, vector<float>> generatePatchesFromBlocks(const vector<float>& blocks, bool generateBases) {
    vector<float> patches;
    vector<float> faceNormals;

    const int vertsPerBlock = 8;
    const int floatsPerVert = 3;
    const int floatsPerBlock = vertsPerBlock * floatsPerVert;

    for (size_t offset = 0; offset + floatsPerBlock <= blocks.size(); offset += floatsPerBlock) {

        // Estrai i vertici
        auto getVert = [&](int i) -> vec3 {
            return vec3(
                blocks[offset + i * 3 + 0],
                blocks[offset + i * 3 + 1],
                blocks[offset + i * 3 + 2]
            );
        };

        vec3 p0 = getVert(0); // -x -y -z
        vec3 p1 = getVert(1); // +x -y -z
        vec3 p2 = getVert(2); // +x -y +z
        vec3 p3 = getVert(3); // -x -y +z
        vec3 p4 = getVert(4); // -x +y -z
        vec3 p5 = getVert(5); // +x +y -z
        vec3 p6 = getVert(6); // +x +y +z
        vec3 p7 = getVert(7); // -x +y +z

        // Tutte le facce ordinate in senso orario rispetto alla normale uscente
        vec3 face0[4] = { p0, p1, p2, p3 }; // bottom (-Y)
        vec3 face1[4] = { p4, p5, p6, p7 }; // top (+Y)
        vec3 face2[4] = { p1, p5, p6, p2 }; // right (+X)
        vec3 face3[4] = { p0, p4, p7, p3 }; // left (-X)
        vec3 face4[4] = { p3, p2, p6, p7 }; // front (+Z)
        vec3 face5[4] = { p0, p1, p5, p4 }; // back (-Z)

        vec3* faces[6] = { face0, face1, face2, face3, face4, face5 };

        vec3 center = (p0 + p6) * 0.5f; // centro del cubo

        for (int f = 0; f < 6; ++f) {
            if (!generateBases) {
                if (f == 0 || f == 1) {
                    continue;
                }
            }

            vec3 v0 = faces[f][0];
            vec3 v1 = faces[f][1];
            vec3 v2 = faces[f][2];

            vec3 normal = normalize(cross(v1 - v0, v2 - v0));

            vec3 faceCenter = (v0 + v1 + v2 + faces[f][3]) * 0.25f;
            vec3 toFace = normalize(faceCenter - center);

            if (dot(normal, toFace) < 0.0f) {
                normal = -normal;
            }

            for (int v = 0; v < 4; ++v) {
                patches.push_back(faces[f][v].x);
                patches.push_back(faces[f][v].y);
                patches.push_back(faces[f][v].z);

                faceNormals.push_back(normal.x);
                faceNormals.push_back(normal.y);
                faceNormals.push_back(normal.z);
            }
        }
    }

    return { patches, faceNormals };
}


tuple<vector<float>, vector<vec3>> generateRoofs(const vector<vec3>& positions, const vector<float>& heights, int subdivisions) {
    float baseSize = 1.2f;
    float roofHeight = 0.2f;
    float shrinkFactor = 0.4f; // Riduzione della base superiore

    vector<float> roofs;
    roofs.reserve(positions.size() * subdivisions * subdivisions * 4 * 3 * 6); // riserva abbondante
    vector<vec3> baseVertices;
    baseVertices.reserve(positions.size() * 8); // 8 vertici per tetto base

    for (size_t i = 0; i < positions.size(); ++i) {
        vec3 roofBaseCenter = positions[i] + vec3(0.0f, heights[i], 0.0f);

        float halfSize = baseSize / 2.0f;
        float shrinkHalfSize = halfSize * shrinkFactor;

        // Vertici base inferiore (non suddivisi)
        vec3 lower[4] = {
            roofBaseCenter + vec3(-halfSize, 0.0f, -halfSize),
            roofBaseCenter + vec3(halfSize, 0.0f, -halfSize),
            roofBaseCenter + vec3(halfSize, 0.0f, halfSize),
            roofBaseCenter + vec3(-halfSize, 0.0f, halfSize)
        };

        // Vertici base superiore (non suddivisi)
        vec3 upper[4] = {
            roofBaseCenter + vec3(-shrinkHalfSize, roofHeight, -shrinkHalfSize),
            roofBaseCenter + vec3(shrinkHalfSize, roofHeight, -shrinkHalfSize),
            roofBaseCenter + vec3(shrinkHalfSize, roofHeight, shrinkHalfSize),
            roofBaseCenter + vec3(-shrinkHalfSize, roofHeight, shrinkHalfSize)
        };

        // Salvo i vertici originali NON suddivisi (8 vertici)
        baseVertices.push_back(lower[0]);
        baseVertices.push_back(lower[1]);
        baseVertices.push_back(lower[2]);
        baseVertices.push_back(lower[3]);
        baseVertices.push_back(upper[0]);
        baseVertices.push_back(upper[1]);
        baseVertices.push_back(upper[2]);
        baseVertices.push_back(upper[3]);

        // Tessellazione dei lati del tetto (come prima)
        for (int f = 0; f < 4; ++f) {
            for (int i_sub = 0; i_sub < subdivisions; ++i_sub) {
                for (int j_sub = 0; j_sub < subdivisions; ++j_sub) {
                    float u0 = float(i_sub) / subdivisions;
                    float v0 = float(j_sub) / subdivisions;
                    float u1 = float(i_sub + 1) / subdivisions;
                    float v1 = float(j_sub + 1) / subdivisions;

                    vec3 pA = mix(mix(lower[f], lower[(f + 1) % 4], u0), mix(upper[f], upper[(f + 1) % 4], u0), v0);
                    vec3 pB = mix(mix(lower[f], lower[(f + 1) % 4], u1), mix(upper[f], upper[(f + 1) % 4], u1), v0);
                    vec3 pC = mix(mix(lower[f], lower[(f + 1) % 4], u1), mix(upper[f], upper[(f + 1) % 4], u1), v1);
                    vec3 pD = mix(mix(lower[f], lower[(f + 1) % 4], u0), mix(upper[f], upper[(f + 1) % 4], u0), v1);

                    roofs.push_back(pA.x); roofs.push_back(pA.y); roofs.push_back(pA.z);
                    roofs.push_back(pD.x); roofs.push_back(pD.y); roofs.push_back(pD.z);
                    roofs.push_back(pC.x); roofs.push_back(pC.y); roofs.push_back(pC.z);
                    roofs.push_back(pB.x); roofs.push_back(pB.y); roofs.push_back(pB.z);
                }
            }
        }

        // Tessellazione base inferiore (come prima)
        for (int i_sub = 0; i_sub < subdivisions; ++i_sub) {
            for (int j_sub = 0; j_sub < subdivisions; ++j_sub) {
                float u0 = float(i_sub) / subdivisions;
                float v0 = float(j_sub) / subdivisions;
                float u1 = float(i_sub + 1) / subdivisions;
                float v1 = float(j_sub + 1) / subdivisions;

                vec3 p00 = mix(mix(lower[0], lower[1], u0), mix(lower[3], lower[2], u0), v0);
                vec3 p10 = mix(mix(lower[0], lower[1], u1), mix(lower[3], lower[2], u1), v0);
                vec3 p11 = mix(mix(lower[0], lower[1], u1), mix(lower[3], lower[2], u1), v1);
                vec3 p01 = mix(mix(lower[0], lower[1], u0), mix(lower[3], lower[2], u0), v1);

                roofs.insert(roofs.end(), {
                    p00.x, p00.y, p00.z,
                    p01.x, p01.y, p01.z,
                    p11.x, p11.y, p11.z,
                    p10.x, p10.y, p10.z
                });
            }
        }

        // Tessellazione base superiore (come prima)
        for (int i_sub = 0; i_sub < subdivisions; ++i_sub) {
            for (int j_sub = 0; j_sub < subdivisions; ++j_sub) {
                float u0 = float(i_sub) / subdivisions;
                float v0 = float(j_sub) / subdivisions;
                float u1 = float(i_sub + 1) / subdivisions;
                float v1 = float(j_sub + 1) / subdivisions;

                vec3 p00 = mix(mix(upper[0], upper[1], u0), mix(upper[3], upper[2], u0), v0);
                vec3 p10 = mix(mix(upper[0], upper[1], u1), mix(upper[3], upper[2], u1), v0);
                vec3 p11 = mix(mix(upper[0], upper[1], u1), mix(upper[3], upper[2], u1), v1);
                vec3 p01 = mix(mix(upper[0], upper[1], u0), mix(upper[3], upper[2], u0), v1);

                roofs.insert(roofs.end(), {
                    p00.x, p00.y, p00.z,
                    p01.x, p01.y, p01.z,
                    p11.x, p11.y, p11.z,
                    p10.x, p10.y, p10.z
                });
            }
        }
    }

    return { roofs, baseVertices };
}


pair<vector<float>, vector<float>> generatePatchesFromRoofs(const vector<float>& roofs, int subdivisions) {
    vector<float> patches;
    vector<float> faceNormals;

    const int floatsPerVert = 3;
    const int quadsPerFace = subdivisions * subdivisions;
    const int totalFaces = 6;
    const int totalQuads = totalFaces * quadsPerFace;
    const int floatsPerQuad = 4 * floatsPerVert;
    const int floatsPerRoof = totalQuads * floatsPerQuad;

    for (size_t offset = 0; offset + floatsPerRoof <= roofs.size(); offset += floatsPerRoof) {
        for (int q = 0; q < totalQuads; ++q) {
            size_t quadOffset = offset + q * floatsPerQuad;
            int faceIndex = q / quadsPerFace;

            // Estrazione dei vertici in ordine CW: p0, p3, p2, p1
            vec3 p0(roofs[quadOffset + 0], roofs[quadOffset + 1], roofs[quadOffset + 2]);
            vec3 p3(roofs[quadOffset + 9], roofs[quadOffset + 10], roofs[quadOffset + 11]);
            vec3 p2(roofs[quadOffset + 6], roofs[quadOffset + 7], roofs[quadOffset + 8]);
            vec3 p1(roofs[quadOffset + 3], roofs[quadOffset + 4], roofs[quadOffset + 5]);

            // Calcolo normale con winding CW
            vec3 normal = normalize(cross(p3 - p0, p1 - p0));

            // Flip base inferiore (deve puntare in basso)
            if (faceIndex == 4 && normal.y > 0.0f) {
                normal = -normal;
            }
            // Flip base superiore (deve puntare in alto)
            else if (faceIndex == 5 && normal.y < 0.0f) {
                normal = -normal;
            }
            // Lati inclinati: punta verso l'esterno
            else if (faceIndex >= 0 && faceIndex <= 3) {
                normal = -normal;
            }

            // Inserimento vertici in ordine CW
            patches.insert(patches.end(), {
                p0.x, p0.y, p0.z,
                p3.x, p3.y, p3.z,
                p2.x, p2.y, p2.z,
                p1.x, p1.y, p1.z
                });

            // Normali duplicate per ogni vertice
            for (int i = 0; i < 4; ++i) {
                faceNormals.insert(faceNormals.end(), { normal.x, normal.y, normal.z });
            }
        }
    }

    return { patches, faceNormals };
}


pair<vector<vec3>, vector<vec3>> generateLampLinesFromBases(const vector<vec3>& basePositions, const vector<vec3>& directions, vector<pair<vec3, vec3>>& verticalRods) {
    vector<vec3> result;
    vector<vec3> lightPositions;

    // Altezza e larghezza costante per tutti i lampioni
    float height = 1.3f;  // valore fisso
    float width = 0.3f;
    float halfWidth = width * 0.5f;

    for (size_t i = 0; i < basePositions.size(); ++i) {
        const vec3& base = basePositions[i];
        const vec3& dir = directions[i];

        float angle = atan2(dir.x, dir.z);

        // Funzione lambda per ruotare un offset attorno a Y
        auto rotateY = [&](const vec3& offset) -> vec3 {
            return vec3(
                offset.x * cos(angle) + offset.z * sin(angle),
                offset.y,
                -offset.x * sin(angle) + offset.z * cos(angle)
            );
        };

        vec3 baseLeft = base + rotateY(vec3(-halfWidth, 0.0f, 0.0f));
        vec3 topLeft = base + rotateY(vec3(-halfWidth, height, 0.0f));
        vec3 topRight = base + rotateY(vec3(halfWidth, height, 0.0f));
        float shortLeg = height * 0.15f;
        vec3 baseRight = base + rotateY(vec3(halfWidth, height - shortLeg, 0.0f));

        // Salvo asta verticale
        verticalRods.push_back({ baseLeft, topLeft });

        // Curva 1: baseLeft - topLeft - topRight - baseRight
        result.insert(result.end(), { baseLeft, topLeft, topRight, baseRight });
        // Curva 2: topLeft - topRight - baseRight - baseRight
        result.insert(result.end(), { topLeft, topRight, baseRight, baseRight });
        // Curva 3: baseLeft - baseLeft - topLeft - topRight
        result.insert(result.end(), { baseLeft, baseLeft, topLeft, topRight });

        lightPositions.push_back(baseRight);
    }

    return { result, lightPositions };
}

