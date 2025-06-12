#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// Texture samplers
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

// Material flags
uniform bool hasTexture;

struct DirLight {
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_POINT_LIGHTS 4
#define NR_SPOT_LIGHTS 4

// Advanced shading parameters
uniform float time;
uniform bool enableAdvancedShading;
uniform float ambientOcclusion;
uniform float roughness;
uniform float metallic;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcAdvancedShading(vec3 baseColor, vec3 normal, vec3 viewDir, vec3 fragPos);
float CalcShadowFactor(vec3 fragPos, vec3 lightPos);

void main()
{    
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
      // Phase 2: point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
    
    // Phase 3: spot lights (now supports 4 spotlights)
    for(int i = 0; i < NR_SPOT_LIGHTS; i++)
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);    
    
    // Phase 4: Advanced shading effects
    if (enableAdvancedShading) {
        result = CalcAdvancedShading(result, norm, viewDir, FragPos);
    }
    
    FragColor = vec4(result, 1.0);
}

// Calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // Get material colors (either from texture or material properties)
    vec3 ambient_color = material.ambient;
    vec3 diffuse_color = material.diffuse;
    vec3 specular_color = material.specular;
    
    if (hasTexture) {
        vec3 textureColor = texture(texture_diffuse1, TexCoord).rgb;
        ambient_color = textureColor;
        diffuse_color = textureColor;
        // Use texture for specular if available, otherwise use material
        if (textureSize(texture_specular1, 0).x > 1) {
            specular_color = texture(texture_specular1, TexCoord).rgb;
        }
    }
    
    // Combine results
    vec3 ambient = light.ambient * ambient_color;
    vec3 diffuse = light.diffuse * diff * diffuse_color;
    vec3 specular = light.specular * spec * specular_color;
    return (ambient + diffuse + specular);
}

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    // Get material colors (either from texture or material properties)
    vec3 ambient_color = material.ambient;
    vec3 diffuse_color = material.diffuse;
    vec3 specular_color = material.specular;
    
    if (hasTexture) {
        vec3 textureColor = texture(texture_diffuse1, TexCoord).rgb;
        ambient_color = textureColor;
        diffuse_color = textureColor;
        // Use texture for specular if available, otherwise use material
        if (textureSize(texture_specular1, 0).x > 1) {
            specular_color = texture(texture_specular1, TexCoord).rgb;
        }
    }
    
    // Combine results
    vec3 ambient = light.ambient * ambient_color;
    vec3 diffuse = light.diffuse * diff * diffuse_color;
    vec3 specular = light.specular * spec * specular_color;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// Calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // Get material colors (either from texture or material properties)
    vec3 ambient_color = material.ambient;
    vec3 diffuse_color = material.diffuse;
    vec3 specular_color = material.specular;
    
    if (hasTexture) {
        vec3 textureColor = texture(texture_diffuse1, TexCoord).rgb;
        ambient_color = textureColor;
        diffuse_color = textureColor;
        // Use texture for specular if available, otherwise use material
        if (textureSize(texture_specular1, 0).x > 1) {
            specular_color = texture(texture_specular1, TexCoord).rgb;
        }
    }
    
    // Combine results
    vec3 ambient = light.ambient * ambient_color;
    vec3 diffuse = light.diffuse * diff * diffuse_color;
    vec3 specular = light.specular * spec * specular_color;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

// Advanced shading with PBR-like effects
vec3 CalcAdvancedShading(vec3 baseColor, vec3 normal, vec3 viewDir, vec3 fragPos) {
    // Enhanced ambient occlusion
    float ao = mix(1.0, ambientOcclusion, 0.5);
    
    // Simple fresnel effect
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 3.0);
    
    // Metallic workflow simulation
    vec3 metallicColor = mix(baseColor, baseColor * 0.04, metallic);
    
    // Roughness-based reflection
    float roughnessFactor = 1.0 - roughness;
    vec3 reflection = reflect(-viewDir, normal);
    
    // Time-based effects for dynamic lighting
    float pulse = 0.8 + 0.2 * sin(time * 2.0);
    
    // Subsurface scattering approximation
    vec3 subsurface = baseColor * 0.1 * max(0.0, dot(normal, viewDir));
    
    // Rim lighting effect
    float rim = 1.0 - dot(viewDir, normal);
    rim = smoothstep(0.6, 1.0, rim);
    vec3 rimColor = vec3(0.2, 0.4, 1.0) * rim * 0.5;
    
    // Combine effects
    vec3 result = baseColor * ao;
    result = mix(result, metallicColor, metallic);
    result += rimColor;
    result += subsurface;
    result *= pulse;
    
    return result;
}

// Simple shadow calculation
float CalcShadowFactor(vec3 fragPos, vec3 lightPos) {
    vec3 lightDir = normalize(lightPos - fragPos);
    float distance = length(lightPos - fragPos);
    
    // Simple distance-based shadow falloff
    float shadow = 1.0 / (1.0 + distance * distance * 0.01);
    
    return shadow;
}
