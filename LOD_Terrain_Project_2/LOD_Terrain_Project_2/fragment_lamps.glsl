#version 460 core

in vec3 gsFragPos;
in vec3 gsNormal;

out vec4 FragColor;

struct PointLight {
    vec3 position;
    vec3 color;
    float power;
};

uniform vec3 ViewPos;
uniform PointLight light;

void main() {
    vec3 norm = normalize(gsNormal);
    vec3 lightDir = normalize(light.position - gsFragPos);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light.color;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.color;

    vec3 viewDir = normalize(ViewPos - gsFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specularStrength = 0.5;
    vec3 specular = specularStrength * spec * light.color;

    vec3 baseColor = vec3(0.6, 0.6, 0.6); // colore giallo chiaro lampada

    vec3 lighting = (ambient + diffuse + specular) * baseColor * light.power;

    FragColor = vec4(lighting, 1.0);
}
