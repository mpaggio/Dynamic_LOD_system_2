#version 460 core

//UNA ESECUZIONE PER OGNI PATCH

layout(vertices = 4) out;

uniform bool useCharacterToTess;
uniform vec3 cameraPosition;
uniform vec3 characterPosition;

const int MIN_TES = 12;
const int MAX_TES = 64;
const float MIN_DIST = 1.0;
const float MAX_DIST = 1.5;

in vec3 normal[];

out vec3 normal_tcs[];

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position; 
    normal_tcs[gl_InvocationID] = normal[gl_InvocationID];
    
    if (gl_InvocationID == 0) {

        vec3 center0 = gl_in[0].gl_Position.xyz + (gl_in[3].gl_Position.xyz - gl_in[0].gl_Position.xyz) / 2.0; //Left
        vec3 center1 = gl_in[1].gl_Position.xyz + (gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz) / 2.0; //Bottom
        vec3 center2 = gl_in[2].gl_Position.xyz + (gl_in[1].gl_Position.xyz - gl_in[2].gl_Position.xyz) / 2.0; //Right
        vec3 center3 = gl_in[3].gl_Position.xyz + (gl_in[2].gl_Position.xyz - gl_in[3].gl_Position.xyz) / 2.0; //Top

        vec3 basePosition = useCharacterToTess ? characterPosition : cameraPosition;
        basePosition.y = 0.0;

        center0.y = 0.0;
        center1.y = 0.0;
        center2.y = 0.0;
        center3.y = 0.0;

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

        // Calcolo della lunghezza dei lati per mantenere i quadrati nelle suddivisioni
        float lengthLeft = length(gl_in[3].gl_Position.xyz - gl_in[0].gl_Position.xyz);
        float lengthBottom = length(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz);
        float lengthRight = length(gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz);
        float lengthTop = length(gl_in[3].gl_Position.xyz - gl_in[2].gl_Position.xyz);

        float tessLevelVertical = max(tes0 / lengthLeft, tes2 / lengthRight);
        float tessLevelHorizontal = max(tes1 / lengthBottom, tes3 / lengthTop);

        float density = (tessLevelVertical + tessLevelHorizontal) * 0.5;

        // Calcoliamo i nuovi livelli di tessellazione moltiplicando la densità per la lunghezza di ciascun lato
        int finalTes0 = int(density * lengthLeft);
        int finalTes1 = int(density * lengthBottom);
        int finalTes2 = int(density * lengthRight);
        int finalTes3 = int(density * lengthTop);

        finalTes0 = clamp(finalTes0, MIN_TES, MAX_TES);
        finalTes1 = clamp(finalTes1, MIN_TES, MAX_TES);
        finalTes2 = clamp(finalTes2, MIN_TES, MAX_TES);
        finalTes3 = clamp(finalTes3, MIN_TES, MAX_TES);

        // Impostiamo i livelli di tessellazione per i bordi
        gl_TessLevelOuter[0] = finalTes0; // Lato sinistro
        gl_TessLevelOuter[1] = finalTes1; // Lato inferiore
        gl_TessLevelOuter[2] = finalTes2; // Lato destro
        gl_TessLevelOuter[3] = finalTes3; // Lato superiore

        // Per gli inner levels, facciamo in modo che siano la media dei bordi opposti
        gl_TessLevelInner[0] = (finalTes1 + finalTes3) / 2; // Direzione orizzontale
        gl_TessLevelInner[1] = (finalTes0 + finalTes2) / 2; // Direzione verticale
    }
}