#version 460 core

layout(vertices = 4) out;

uniform vec3 cameraPosition;
uniform vec3 characterPosition;
uniform bool useCharacterToTess;

const float minDist = 1.0;
const float maxDist = 5.0;
const int minTessLevel = 4;
const int maxTessLevel = 64;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
        vec3 basePosition = useCharacterToTess ? characterPosition : cameraPosition;

        float distSum = 0.0;
        for (int i = 0; i < 4; ++i) {
            distSum += length(basePosition - gl_in[i].gl_Position.xyz);
        }

        float avgDist = distSum / 4.0;
        float tessLevel = mix(float(maxTessLevel), float(minTessLevel), clamp((avgDist - minDist) / (maxDist - minDist), 0.0, 1.0));

        gl_TessLevelOuter[0] = 1.0;
        gl_TessLevelOuter[1] = tessLevel;
    }
}
