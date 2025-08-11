#version 460 core

//UNA ESECUZIONE PER OGNI PATCH

layout(vertices = 4) out;

uniform bool useCharacterToTess;
uniform vec3 cameraPosition;
uniform vec3 characterPosition;

const int MIN_TES = 6;
const int MAX_TES = 44;
const float MIN_DIST = 1.0;
const float MAX_DIST = 1.5;

in vec4 vDisplace[];

out vec4 tcDisplace[];

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcDisplace[gl_InvocationID] = vDisplace[gl_InvocationID];

    if (gl_InvocationID == 0) {

        vec3 center0 = gl_in[0].gl_Position.xyz + (gl_in[3].gl_Position.xyz - gl_in[0].gl_Position.xyz) / 2.0; //Left
        vec3 center1 = gl_in[1].gl_Position.xyz + (gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz) / 2.0; //Bottom
        vec3 center2 = gl_in[2].gl_Position.xyz + (gl_in[1].gl_Position.xyz - gl_in[2].gl_Position.xyz) / 2.0; //Right
        vec3 center3 = gl_in[3].gl_Position.xyz + (gl_in[2].gl_Position.xyz - gl_in[3].gl_Position.xyz) / 2.0; //Top

        vec3 basePosition;
        if (useCharacterToTess) {
            basePosition = characterPosition;
        }
        else {
            basePosition = cameraPosition;
        }

        //Calcolo delle distanze fra i centri e la camer
        float dist0 = length(basePosition - center0);
        float dist1 = length(basePosition - center1);
        float dist2 = length(basePosition - center2);
        float dist3 = length(basePosition - center3);

        //Calcolo dei livelli di tassellizzazione
        int tes0 = (dist0 < MIN_DIST) ? MAX_TES : ((dist0 > MAX_DIST) ? MIN_TES : int(mix(MAX_TES, MIN_TES, clamp(dist0 / MAX_DIST, 0.0, 1.0))));
        int tes1 = (dist1 < MIN_DIST) ? MAX_TES : ((dist1 > MAX_DIST) ? MIN_TES : int(mix(MAX_TES, MIN_TES, clamp(dist1 / MAX_DIST, 0.0, 1.0))));
        int tes2 = (dist2 < MIN_DIST) ? MAX_TES : ((dist2 > MAX_DIST) ? MIN_TES : int(mix(MAX_TES, MIN_TES, clamp(dist2 / MAX_DIST, 0.0, 1.0))));
        int tes3 = (dist3 < MIN_DIST) ? MAX_TES : ((dist3 > MAX_DIST) ? MIN_TES : int(mix(MAX_TES, MIN_TES, clamp(dist3 / MAX_DIST, 0.0, 1.0))));

        //Indica la suddivisione interna del patch (concretamente indica il numero di linee parallele a quelle dei lati della figura originale)
        gl_TessLevelInner[0] = max(tes1, tes3); //Lato interno in alto
        gl_TessLevelInner[1] = max(tes0, tes2); //Lato interno in basso

        //Indica la suddivisione sui bordi esterni del patch
        gl_TessLevelOuter[0] = tes0; //Lato di sinistra
        gl_TessLevelOuter[1] = tes1; //Lato in basso
        gl_TessLevelOuter[2] = tes2; //Lato a destra
        gl_TessLevelOuter[3] = tes3; //Lato in alto
    }
}