#pragma once
#include "lib.h"
#include "strutture.h"

vector<float> generateSkyboxCube();
vector<float> simplePlane(int division, float width);
pair<vector<vec3>, vector<vec3>> generateSphericalBasesFromPositions(const vector<vec3>& basePositions);
vector<float> generatePatches(const vector<float>& plane, int division);
tuple<vector<float>, vector<float>, vector<vec3>> generateBlocks(const vector<vec3>& positions, int subdivisions, bool isHedge);
pair<vector<float>, vector<float>> generatePatchesFromBlocks(const vector<float>& blocks, bool generateBases);
tuple<vector<float>, vector<vec3>> generateRoofs(const vector<vec3>& positions, const vector<float>& heights, int subdivisions);
pair<vector<float>, vector<float>> generatePatchesFromRoofs(const vector<float>& roofs, int subdivisions);
pair<vector<vec3>, vector<vec3>> generateLampLinesFromBases(const vector<vec3>& basePositions);