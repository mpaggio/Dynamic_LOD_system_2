#version 460 core

in vec4 gs_worldPos; //Input proveniente dal Geometry Shader
in vec3 gs_normal;

out vec4 FragColor; //Output del Fragment Shader, ovvero il colore finale del pixel (variabile mandata direttamente al frame buffer)

struct PointLight {
    vec3 position;
    vec3 color;
    float power;
};

uniform vec3 ViewPos;
uniform PointLight light;

void main() {
    vec3 norm = normalize(gs_normal);                
    vec3 lightDir = normalize(light.position - gs_worldPos.xyz); 

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light.color;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.color;

    vec3 viewDir = normalize(ViewPos - gs_worldPos.xyz);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specularStrength = 0.5;
    vec3 specular = specularStrength * spec * light.color;

    vec3 baseColor = vec3(1.0, 0.8, 0.6);

    vec3 lighting = (ambient + diffuse + specular) * baseColor * light.power;

    FragColor = vec4(lighting, 1.0);
}