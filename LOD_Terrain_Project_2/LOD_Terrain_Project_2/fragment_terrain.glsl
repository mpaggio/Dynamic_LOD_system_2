#version 460 core

in vec2 tesUV_gs;
in vec3 normal_gs;
in vec3 worldPos_gs;

out vec4 FragColor;

struct PointLight {
    vec3 position;
    vec3 color;
    float power;
};

uniform vec3 ViewPos;
uniform PointLight light;

uniform sampler2D texture0; //color

void main() {
    vec3 norm = normalize(normal_gs);

    vec3 lightDir = normalize(light.position - worldPos_gs);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light.color;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.color;

    vec3 viewDir = normalize(ViewPos - worldPos_gs);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specularStrength = 0.5;
    vec3 specular = specularStrength * spec * light.color;

    vec3 baseColor = texture(texture0, tesUV_gs).rgb;

    vec3 lighting = (ambient + diffuse + specular) * baseColor * light.power;

    FragColor = vec4(lighting, 1.0);
}