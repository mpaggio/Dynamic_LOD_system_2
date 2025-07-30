#pragma once
#include "lib.h"
#include "strutture.h"

float randomFloat(float min, float max);
vec3 randomPosition(float L);
pair<vector<vec3>, vector<vec3>> generateNonOverlappingPositions(const vector<float>& plane, int division, float width, int numBlocks, int numHedges);