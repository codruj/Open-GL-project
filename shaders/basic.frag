#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

// Light properties
uniform vec3 pointLightPos;    // Position of the point light
uniform vec3 pointLightColor;  // Color of the point light
uniform vec3 lightDir;         // Directional light direction
uniform vec3 lightColor;       // Directional light color

// Enable/disable lights
uniform bool pointLightEnabled;
uniform bool directionalLightEnabled;

// Fog
uniform vec3 fogColor;
uniform float fogDensity;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

// Light components
vec3 ambient;
float ambientStrength = 0.3f;  // Subtle ambient light
vec3 diffuse;
vec3 specular;
float specularStrength = 0.6f; // Specular light strength

uniform float lightningIntensity;

void computePointLight() {
    if (pointLightEnabled) {
        vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
        vec3 normalEye = normalize(normalMatrix * fNormal);

        vec3 lightDir = normalize(pointLightPos - fPosition);

        // Ambient component
        ambient = ambientStrength * pointLightColor;

        // Diffuse component
        float diff = max(dot(normalEye, lightDir), 0.0);
        diffuse = diff * pointLightColor;

        // Specular component
        vec3 viewDir = normalize(-fPosEye.xyz);
        vec3 reflectDir = reflect(-lightDir, normalEye);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        specular = specularStrength * spec * pointLightColor;

        // Distance to the light source
        float distanceToLight = length(pointLightPos - fPosition);

        // Extreme falloff (adjust as needed)
        float intensity = 100.0 / (0.1 + pow(distanceToLight, 20.0)); // r^20 falloff
        intensity = clamp(intensity, 0.0, 0.4); // Limit maximum intensity

        // Adjust light contribution based on falloff
        ambient *= intensity;
        diffuse *= intensity;
        specular *= intensity;
    } else {
        ambient = vec3(0.0);
        diffuse = vec3(0.0);
        specular = vec3(0.0);
    }
}

void computeDirectionalLight(vec3 normalEye, vec3 viewDir) {
    if (directionalLightEnabled) {
        vec3 lightDirEye = normalize((view * vec4(-lightDir, 0.0)).xyz);

        // Ambient component
        ambient += ambientStrength * lightColor;

        // Diffuse component
        float diff = max(dot(normalEye, lightDirEye), 0.0);
        diffuse += diff * lightColor;

        // Specular component
        vec3 reflectDir = reflect(-lightDirEye, normalEye);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        specular += specularStrength * spec * lightColor;
    }
}

void main() {
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 viewDir = normalize(-fPosEye.xyz);

    // Reset light components
    ambient = vec3(0.0);
    diffuse = vec3(0.0);
    specular = vec3(0.0);

    // Compute lights
    computePointLight();
    computeDirectionalLight(normalEye, viewDir);

    // Combine light contributions
    vec3 lightingColor = ambient + diffuse + specular;

    // Sample textures
    vec3 textureColor = texture(diffuseTexture, fTexCoords).rgb;

    // Final color
    vec3 finalColor = lightingColor * textureColor;

    // Apply fog
    float distance = length((view * vec4(fPosition, 1.0f)).xyz);
    float fogFactor = exp(-fogDensity * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    finalColor = mix(fogColor, finalColor, fogFactor) * lightningIntensity;
    fColor = vec4(finalColor, 1.0f);
}
