#version 460 core

layout(vertices = 3) out; // Patch di 3 vertici per triangoli

in vec3 vsCenter[]; //Dal vertex shader
out vec3 tcCenter[]; //Al TES

uniform vec3 cameraPosition;
uniform vec3 characterPosition;
uniform bool useCharacterToTess;

const int MIN_TES = 1;
const int MAX_TES = 16;
const float MAX_DIST = 6.0;

void main() {
    // Passa i vertici in output senza modifiche
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcCenter[gl_InvocationID] = vsCenter[gl_InvocationID];

    // Solo un thread decide i livelli di tessellazione
    if (gl_InvocationID == 0) {
        vec3 center = vsCenter[0];
        vec3 basePosition = useCharacterToTess ? characterPosition : cameraPosition;

        float dist = length(basePosition - center);
        float lodFactor = clamp(dist / MAX_DIST, 0.0, 1.0);
        float rawLevel = mix(MAX_TES, MIN_TES, lodFactor);
        int tessLevel = clamp(int(rawLevel), MIN_TES, MAX_TES);

        //Outer
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        
        //Inner
        gl_TessLevelInner[0] = tessLevel;
    }
}
