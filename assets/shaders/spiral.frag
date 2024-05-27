
//////////// Rim Light
#version 330 core

in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 viewPos;     // 相机位置
uniform vec3 rimColor;    // 边缘颜色
uniform float rimWidth;   // 设置边缘照明宽度
uniform float rimStrength;  // 设置边缘照明强度
uniform float time;         // 累计时间

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

//uniform Material material;
uniform Light light;

void main()
{
    float ambientStrength = 0.8;
    // vec3 ambientColor = light.ambient * material.ambient;
    vec3 ambientColor = light.ambient * ambientStrength;
	
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);

    // diffuse
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    //vec3 diffuseColor = light.diffuse * (diff * material.diffuse);

    // float specularStrength = 0.5;
    // float shininess = 32.0;
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    //vec3 specularColor = light.specular * (spec * material.specular);

    float rim = rimStrength * max(0.0, 1.0 - dot(Normal, viewDir)) / rimWidth;
    //FragColor = vec4(ambientColor + diffuseColor + specularColor + rim * rimColor, 1.0);

    FragColor = vec4(ambientColor * (sin(time * 5) + 0.5f) + rim * rimColor, 1.0);
}


//////////////// Other
// #version 330 core

// in VS_OUT 
// {
    // vec3 N;
    // vec3 L;
    // vec3 V;
// } fs_in;

// vec3 diffuseAlbedo = vec3(0.780392, 0.568627, 0.113725);
// vec3 specularAlbedo = vec3(0.1, 0.1, 0.1);

// float shininess = 128.0;
// float rimPower = 3.0;

// // uniform int specularOn = 1;
// // uniform int rimLightOn = 1;

// out vec4 out_color;

// const vec4 ambient = vec4(0.329412, 0.223529, 0.027451, 0.8);

// vec3 rimLight(vec3 N, vec3 V) {
    // vec3 rimColor = vec3(0.0, 1.0, 0.0);

    // float f = 1.0 - dot(N, V);

    // f = smoothstep(0.0, 1.0, f);

    // f = pow(f, rimPower);

    // return f * rimColor;
// }

// void main() {
    // vec3 N = normalize(fs_in.N);
    // vec3 L = normalize(fs_in.L);
    // vec3 V = normalize(fs_in.V);
    // vec3 R = reflect(-L, N);

    // vec3 diffuse = max(dot(N, L), 0.0) * diffuseAlbedo;

    // //vec3 specular = vec3(0.0);
	// vec3 specular = pow(max(dot(R, V), 0.0), shininess) * specularAlbedo;
	// out_color = ambient + vec4(rimLight(N, V), 1.0);
    // // if(specularOn == 1) 
	// // {
        // // specular = pow(max(dot(R, V), 0.0), shininess) * specularAlbedo;
    // // }

    // // if(rimLightOn == 1) 
	// // {
        // // // out_color = ambient + vec4(diffuse + specular + rimLight(N, V), 1.0);
		// // out_color = vec4(rimLight(N, V), 1.0);
    // // } 
	// // else 
	// // {
        // // out_color = ambient + vec4(diffuse + specular, 1.0);
    // // }
// }


