#pragma once
#include "lib.h"
#include "strutture.h"

float randomFloat(float min, float max);
int randomInt(int min, int max);
vec3 randomPosition(float L);
tuple<vector<vec3>, vector<vec3>, vector<vec3>, vector<vec3>> generateCityPositions(
    const vector<float>& plane,
    const vector<bool>& isRoad,
    int division,
    int numBlocks,
    int numHedges,
    int mapWidth
);
pair<vec3, vec3> getBoundingBox(const vector<float>& vertices);
pair<vec3, vec3> getBoundingBox(const vector<vec3>& vertices);
bool checkCollision(const vec3& playerPos, const vec3& minBB, const vec3& maxBB);