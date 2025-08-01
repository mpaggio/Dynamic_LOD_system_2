#pragma once
#include "lib.h"
#include "strutture.h"

float randomFloat(float min, float max);
int randomInt(int min, int max);
vec3 randomPosition(float L);
tuple<vector<vec3>, vector<vec3>, vector<vec3>> generateNonOverlappingPositions(
	const vector<float>& plane,
	int division,
	int numBlocks,
	int numHedges,
	int numLamps
);