#include "utilities.h"

// Funzione per generare float casuale tra min e max
float randomFloat(float min, float max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<> dis(min, max);
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

pair<vector<vec3>, vector<vec3>> generateNonOverlappingPositions(const vector<float>& plane, int division, float width, int numBlocks, int numHedges) {
    float cellSize = 1.0f; // Supponiamo che ogni cella sia larga 1.0f (modifica se diverso)
    int cellsPerBlock = ceil(width / cellSize); // Quante celle sono occupate da un blocco
    int padding = cellsPerBlock / 2;

    vector<vec3> positions;
    vector<vector<bool>> occupied(division + 1, vector<bool>(division + 1, false));

    mt19937 rng(std::random_device{}());
    uniform_int_distribution<int> dist(padding, division - padding - 1); // Evita i bordi

    int total = numBlocks + numHedges;
    int attempts = 0;
    const int maxAttempts = total * 30;

    while ((int)positions.size() < total && attempts < maxAttempts) {
        int row = dist(rng);
        int col = dist(rng);

        // Verifica che tutte le celle attorno siano libere
        bool canPlace = true;
        for (int i = -padding; i <= padding && canPlace; ++i) {
            for (int j = -padding; j <= padding; ++j) {
                int r = row + i;
                int c = col + j;
                if (r < 0 || r >= division + 1 || c < 0 || c >= division + 1 || occupied[r][c]) {
                    canPlace = false;
                    break;
                }
            }
        }

        if (canPlace) {
            size_t idx = (row * (division + 1) + col) * 3;
            float x = plane[idx + 0];
            float z = plane[idx + 1];
            float y = plane[idx + 2];

            positions.emplace_back(x, y, z);

            // Marca tutte le celle attorno come occupate
            for (int i = -padding; i <= padding; ++i) {
                for (int j = -padding; j <= padding; ++j) {
                    occupied[row + i][col + j] = true;
                }
            }
        }

        attempts++;
    }

    vector<vec3> blockPositions(positions.begin(), positions.begin() + numBlocks);
    vector<vec3> hedgePositions(positions.begin() + numBlocks, positions.end());

    return { blockPositions, hedgePositions };
}
