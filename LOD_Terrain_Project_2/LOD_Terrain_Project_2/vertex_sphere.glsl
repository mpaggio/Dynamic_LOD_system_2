#version 460 core

layout(location = 0) in vec3 aPos; //Attributo posizione (vettore a 3 componenti)
layout(location = 1) in vec3 inCenter; //Attributo centro della sfera corrispondente (vettore a 3 componenti)

out vec3 vsCenter;

void main() {
    vsCenter = vec3(inCenter); //Assegno il valore alla variabile di output che verrà mandata al TCS
    gl_Position = vec4(aPos, 1.0); //Trasforma l'attributo in un vettore a 4 componenti (le due componenti già presenti in aPos, il livello di profondità e il valore per essere una coordinata omogenea)
}