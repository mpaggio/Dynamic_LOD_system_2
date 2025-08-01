#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec4 worldPos[]; //Input dal Tessellation Evaluation Shader
in vec3 te_normal[];

out vec4 gs_worldPos; //Output per il Fragment Shader
out vec3 gs_normal;

uniform mat4 view; //Simula la camera (sposta e ruota il mondo per simulare il punto di vista dell'osservatore)
uniform mat4 proj; //converte le coordinate da mondo 3D alla visione 2D (matrice di proiezione)

void main() {

    vec4 center = (worldPos[0] + worldPos[1] + worldPos[2]) / 3.0; //Calcola il centro del triangolo

    for (int i = 0; i < 3; ++i) {
        vec4 offset = normalize(worldPos[i] - center);
        vec4 newWorldPos = worldPos[i];

        gl_Position = proj * view * newWorldPos; //Trasforma da coordinate 3D del mondo nelle coordinate di vista della telecamera e infine in coordinate in 2D
        gs_worldPos = newWorldPos;
        gs_normal = normalize(te_normal[i]);

        EmitVertex();
    }
    EndPrimitive();

}
