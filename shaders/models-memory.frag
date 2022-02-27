#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormals;
layout(location = 3) in vec4 eye;
layout(location = 4) in vec4 light;

struct ComponentProperties {
    int ambientTexture;
    int diffuseTexture;
    int specularTexture;
    int normalTexture;
    vec3 ambientColor;
    float emissiveFactor;
    vec3 diffuseColor;
    float opacity;
    vec3 specularColor;
    float shininess;
};

layout(location = 5) in flat ComponentProperties compProperties;

layout(binding = 2) uniform sampler2D samplers[25];

layout(location = 0) out vec4 outColor;

void main() {
    bool hasTextures =
        compProperties.ambientTexture != -1 || compProperties.diffuseTexture != -1 ||
        compProperties.specularTexture != -1 || compProperties.normalTexture != -1;

    vec3 emissiveContribution = vec3(1) * compProperties.emissiveFactor;
    
    // ambientContribution
    vec3 ambientContribution = compProperties.ambientColor;
    
    // diffuseContribution
    vec3 diffuseContribution = compProperties.diffuseColor;
    
    // specularContribution
    vec3 specularContribution = compProperties.specularColor;

    // global light source
    vec3 lightDirection = normalize(vec3(light) - fragPosition);

    // normals adjustment if normal texture is present
    vec3 normals = fragNormals;
    if (compProperties.normalTexture != -1) {
        normals = texture(samplers[compProperties.normalTexture], fragTexCoord).rgb;
        normals = normalize(normals * 2.0 - 1.0);
    }
    
    // diffuse multiplier based on normals
    float diffuse = max(dot(normals, lightDirection), 0);

    // specular multiplier based on normals and eye direction
    vec3 eyeDirection = normalize(vec3(eye) - fragPosition);
    vec3 halfDirection = normalize(lightDirection + vec3(eye));
    float specular = pow(max(dot(normals, halfDirection), 0), compProperties.shininess);
    
    if (hasTextures) {
        // ambience
        if (compProperties.ambientTexture != -1) {
            ambientContribution = texture(samplers[compProperties.ambientTexture], fragTexCoord).rgb;
        }
        
        // diffuse
        if (compProperties.diffuseTexture != -1) {
            diffuseContribution = texture(samplers[compProperties.diffuseTexture], fragTexCoord).rgb;
        }
        
        // sepcular
        if (compProperties.specularTexture != -1) {
            specularContribution = texture(samplers[compProperties.specularTexture], fragTexCoord).rgb;
        }
    }
    
    outColor = vec4(
        mix((emissiveContribution + ambientContribution + vec3(diffuse)) * diffuseContribution.rgb, 
        vec3(specular) * specularContribution.rgb, 0.40), compProperties.opacity);
}
