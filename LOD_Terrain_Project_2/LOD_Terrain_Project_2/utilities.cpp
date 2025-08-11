#include "utilities.h"

// Funzione per generare float casuale tra min e max
float randomFloat(float min, float max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

int randomInt(int min, int max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

// Genera posizione random dentro quadrato di lato L
vec3 randomPosition(float L) {
    return vec3(
        randomFloat(0, L), // solo coordinate positive in X
        randomFloat(-L, 0), // y rimane come preferisci (ad esempio altezza)
        randomFloat(L / 5, L / 2) // coordinate negative in Z (perché la mappa va verso -Z)
    );
}

tuple<vector<vec3>, vector<vec3>, vector<vec3>, vector<vec3>> generateCityPositions(const vector<float>& plane, 
    const vector<bool>& isRoad, int division, int numBlocks, int numHedges, int mapWidth) {
    float blockBaseSize = 1.0f;    // dimensione base case
    float hedgeBaseSize = 0.5f;    // dimensione base siepi (modifica se vuoi)

    // Calcolo cellSize da plane (dato da width/division)
    float width = mapWidth; // o ricavalo da parametri esterni se possibile
    float cellSize = width / division;

    vector<vector<bool>> occupied(division + 1, vector<bool>(division + 1, false));
    mt19937 rng(std::random_device{}());
    uniform_int_distribution<int> dist(1, division - 2);

    // Funzione generale per controllare area di occupazione
    auto canPlaceHere = [&](int row, int col, bool requireGrass, bool requireRoadEdge, float baseSize) {
        int halfCells = (int)ceil((baseSize / 2.0f) / cellSize);

        for (int i = -halfCells; i <= halfCells; ++i) {
            for (int j = -halfCells; j <= halfCells; ++j) {
                int r = row + i;
                int c = col + j;
                if (r < 0 || r > division || c < 0 || c > division) return false;

                size_t idxCell = r * (division + 1) + c;
                if (requireGrass && isRoad[idxCell]) return false;
                if (requireRoadEdge) {
                    if (!isRoad[idxCell]) return false;

                    // controlla vicino erba
                    bool nearGrass = false;
                    for (int di = -1; di <= 1 && !nearGrass; ++di) {
                        for (int dj = -1; dj <= 1; ++dj) {
                            int rr = r + di;
                            int cc = c + dj;
                            if (rr >= 0 && rr <= division && cc >= 0 && cc <= division) {
                                if (!isRoad[rr * (division + 1) + cc]) nearGrass = true;
                            }
                        }
                    }
                    if (!nearGrass) return false;
                }

                if (occupied[r][c]) return false;
            }
        }
        return true;
    };

    auto markOccupied = [&](int row, int col, float baseSize) {
        int halfCells = (int)ceil((baseSize / 2.0f) / cellSize);

        for (int i = -halfCells; i <= halfCells; ++i) {
            for (int j = -halfCells; j <= halfCells; ++j) {
                int r = row + i;
                int c = col + j;
                if (r >= 0 && r <= division && c >= 0 && c <= division)
                    occupied[r][c] = true;
            }
        }
    };

    vector<vec3> blockPositions;
    vector<vec3> hedgePositions;
    vector<vec3> lampPositions;
    vector<vec3> lampDirections;

    int attempts = 0;
    while ((int)blockPositions.size() < numBlocks && attempts < numBlocks * 50) {
        int row = dist(rng);
        int col = dist(rng);
        if (canPlaceHere(row, col, true, false, blockBaseSize)) {
            size_t idx = (row * (division + 1) + col) * 3;
            blockPositions.emplace_back(plane[idx + 0], plane[idx + 2], plane[idx + 1]);
            markOccupied(row, col, blockBaseSize);
        }
        attempts++;
    }

    attempts = 0;
    while ((int)hedgePositions.size() < numHedges && attempts < numHedges * 50) {
        int row = dist(rng);
        int col = dist(rng);
        if (canPlaceHere(row, col, true, false, hedgeBaseSize)) {
            size_t idx = (row * (division + 1) + col) * 3;
            hedgePositions.emplace_back(plane[idx + 0], plane[idx + 2], plane[idx + 1]);
            markOccupied(row, col, hedgeBaseSize);
        }
        attempts++;
    }

    const int lampInterval = 3;
    for (int row = 0; row <= division; ++row) {
        for (int col = 0; col <= division; ++col) {

            size_t idx = row * (division + 1) + col;

            // Controllo che il vertice sia su una strada e che la posizione non sia occupata
            if (!isRoad[idx] || occupied[row][col]) {
                continue;
            }

            // Controllo se il vertice è un bordo di strada, cioè ha almeno un vicino ortogonale che è erba
            bool isRoadEdge = false;
            int edgeDirX = 0;
            int edgeDirZ = 0;

            if (row > 0 && !isRoad[(row - 1) * (division + 1) + col]) { //sotto
                isRoadEdge = true;
                edgeDirZ = -1;
            }
            else if (row < division && !isRoad[(row + 1) * (division + 1) + col]) { //sopra
                isRoadEdge = true;
                edgeDirZ = 1;
            }
            else if (col > 0 && !isRoad[row * (division + 1) + (col - 1)]) { //sinistra
                isRoadEdge = true;
                edgeDirX = -1;
            }
            else if (col < division && !isRoad[row * (division + 1) + (col + 1)]) { //destra
                isRoadEdge = true;
                edgeDirX = 1;
            }

            // Solo se è bordo e posizione rispettando lampInterval
            if (!isRoadEdge || ((row + col) % lampInterval != 0)) {
                continue;
            }

            // Prendo posizione vertice sulla strada
            size_t plane_idx = idx * 3;
            vec3 pos = { plane[plane_idx + 0], plane[plane_idx + 2], plane[plane_idx + 1] };

            // Direzione lampione orientata verso l’interno della strada (opposta all’erba)
            vec3 dir = { 0.0f, 0.0f, 0.0f };
            dir.x = -edgeDirX;
            dir.z = -edgeDirZ;

            vec3 rotatedDir;
            if (dir.z != 0.0f) {
                rotatedDir.x = dir.z;
                rotatedDir.z = -dir.x;
            }
            else if (dir.x != 0.0f) {
                rotatedDir.x = -dir.z;
                rotatedDir.z = dir.x;
            }
            else {
                rotatedDir = dir; // fallback, direzione zero
            }

            if (edgeDirX == 1 || edgeDirZ == 1) {
                pos.x -= rotatedDir.z * cellSize;
                pos.z += rotatedDir.x * cellSize;
            }

            lampPositions.push_back(pos);
            markOccupied(row, col, 0.0f);
            lampDirections.push_back(rotatedDir);
        }
    };

    return { blockPositions, hedgePositions, lampPositions, lampDirections };
}



pair<vec3, vec3> getBoundingBox(const vector<float>& vertices) {
    vec3 minPoint(numeric_limits<float>::max());
    vec3 maxPoint(numeric_limits<float>::lowest());

    for (size_t i = 0; i < vertices.size(); i += 3) {
        float x = vertices[i];
        float y = vertices[i + 1];
        float z = vertices[i + 2];

        if (x < minPoint.x) minPoint.x = x;
        if (y < minPoint.y) minPoint.y = y;
        if (z < minPoint.z) minPoint.z = z;

        if (x > maxPoint.x) maxPoint.x = x;
        if (y > maxPoint.y) maxPoint.y = y;
        if (z > maxPoint.z) maxPoint.z = z;
    }

    return { minPoint, maxPoint };
}



pair<vec3, vec3> getBoundingBox(const vector<vec3>& vertices) {
    vec3 minPoint(numeric_limits<float>::max());
    vec3 maxPoint(numeric_limits<float>::lowest());

    for (const auto& v : vertices) {
        if (v.x < minPoint.x) minPoint.x = v.x;
        if (v.y < minPoint.y) minPoint.y = v.y;
        if (v.z < minPoint.z) minPoint.z = v.z;

        if (v.x > maxPoint.x) maxPoint.x = v.x;
        if (v.y > maxPoint.y) maxPoint.y = v.y;
        if (v.z > maxPoint.z) maxPoint.z = v.z;
    }

    minPoint -= vec3(0.05, 0.0, 0.05);
    maxPoint += vec3(0.05, 0.0, 0.05);

    return { minPoint, maxPoint };
}



bool checkCollision(const vec3& playerPos, const vec3& minBB, const vec3& maxBB) {
    float offset = 0.1f;
    return (playerPos.x >= minBB.x - offset && playerPos.x <= maxBB.x + offset) &&
        (playerPos.y >= minBB.y - offset && playerPos.y <= maxBB.y + offset) &&
        (playerPos.z >= minBB.z - offset && playerPos.z <= maxBB.z + offset);
}