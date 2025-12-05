#version 330 core

in vec3 worldSpacePosition;
in vec3 worldSpaceNormal;
in vec4 materialAmbient;
in vec4 materialDiffuse;
in vec4 materialSpecular;
in float materialShininess;

uniform float ka;
uniform float kd;
uniform float ks;

uniform vec3 cameraPosition;

uniform int lightCount;

struct Light {

    int type; // 0 - Directional, 1 - Point, 2 - Spotlight

    vec4 color;
    vec3 function;

    vec4 position;
    vec4 direction;

    float penumbra;
    float angle;

};

uniform Light lights[8];

out vec4 color;

vec4 calculateDirectionalLighting(Light light, vec3 normal, vec3 cameraDirection) {

    vec3 lightDirection = normalize(vec3(-light.direction));

    // Diffuse Calculations --
    float diffuseIntensity = max(dot(normal, lightDirection), 0.0);
    vec4 diffuse = kd * materialDiffuse * diffuseIntensity;

    // Specular Calculations --
    vec3 reflectDirection = reflect(-lightDirection, normal);

    float specularIntensity = (materialShininess == 0) ? 1 : pow(max(dot(cameraDirection, reflectDirection), 0.0), materialShininess);
    vec4 specular = ks * materialSpecular * specularIntensity;

    return light.color * (diffuse + specular);
}

vec4 calculatePointLighting(Light light, vec3 normal, vec3 cameraDirection) {

    vec3 lightDirection = normalize(vec3(light.position) - worldSpacePosition);
    float distance = length(vec3(light.position) - worldSpacePosition);

    // Attenuation Caclulations --
    float attenuation = 1.0f / (light.function.x +
                          distance * light.function.y +
                          (distance * distance) * light.function.z);

    attenuation = min(1.0f, attenuation);

    // Diffuse Calculations --
    float diffuseIntensity = max(dot(normal, lightDirection), 0.0);
    vec4 diffuse = kd * materialDiffuse * diffuseIntensity;

    // Specular Calculations --
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float specularIntensity = (materialShininess == 0) ? 1 : pow(max(dot(cameraDirection, reflectDirection), 0.0), materialShininess);
    vec4 specular = ks * materialSpecular * specularIntensity;

    return (light.color * attenuation) * (diffuse + specular);

}

vec4 calculateSpotLighting(Light light, vec3 normal, vec3 cameraDirection) {

    vec3 lightDirection = normalize(vec3(light.position) - worldSpacePosition);
    float distance = length(vec3(light.position) - worldSpacePosition);

    // Attenuation Calculations --
    float attenuation = 1.0 / (light.function.x +
                               distance * light.function.y +
                               (distance * distance) * light.function.z);

    attenuation = min(1.0, attenuation);


    // Falloff Calculations --
    float falloff;

    float inner = light.angle - light.penumbra;
    float outer = light.angle;

    float theta = acos(dot(normalize(vec3(light.direction)),
                                     normalize(worldSpacePosition - vec3(light.position))));

    if (theta <= inner) {

        falloff = 1.0f;

    } else if (theta <= light.angle) {

        float a = (theta - inner) / (light.angle - inner);
        falloff = 1.0f - (-2.0f * (a * a * a) + 3.0f * (a * a));

    } else {

        falloff = 0.0f;

    }

    // Diffuse Calculations --
    float diffuseIntensity = max(dot(normal, lightDirection), 0.0);
    vec4 diffuse = kd * materialDiffuse * diffuseIntensity;

    // Specular Calculations --
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float specularIntensity = (materialShininess == 0) ? 1 : pow(max(dot(cameraDirection, reflectDirection), 0.0), materialShininess);
    vec4 specular = ks * materialSpecular * specularIntensity;

    return (attenuation * light.color * falloff) * (diffuse + specular);

}

void main() {

    vec3 normal = normalize(worldSpaceNormal);
    vec3 directionToCamera = normalize(cameraPosition - worldSpacePosition);

    vec4 illumination = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    illumination += ka * materialAmbient;

    for (int i = 0; i < lightCount; i++) {

        if (lights[i].type == 0) illumination += calculateDirectionalLighting(lights[i], normal, directionToCamera);
        else if (lights[i].type == 1) illumination += calculatePointLighting(lights[i], normal, directionToCamera);
        else illumination += calculateSpotLighting(lights[i], normal, directionToCamera);

    }

    color = vec4(illumination.rgb, 1.0);

}
