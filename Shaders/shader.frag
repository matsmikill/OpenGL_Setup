#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform bool useTexture;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
            
    vec3 result;
    if (useTexture) {
        vec4 texColor = texture(texture1, TexCoord);
        result = (ambient + diffuse) * texColor.rgb;
    } else {
        result = (ambient + diffuse) * objectColor;
    }
    
    FragColor = vec4(result, 1.0);
}