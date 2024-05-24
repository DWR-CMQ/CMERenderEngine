#version 330 core

in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

//uniform vec3 lightPos;
vec3 lightPos = vec3(0.0f, 0.0f, -10.0f);
uniform vec3 viewPos;
uniform vec3 rimlight;    // 边缘光
uniform vec3 ambient;     // 图形光

void main()
{
    float ambientStrength = 0.8;
    vec3 ambientColor = ambientStrength * ambient;
	
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);

    float specularStrength = 0.5;
    float shininess = 32.0;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    float rimStrength = 2.0; // 设置边缘照明强度
    float rimWidth = 1.0; // 设置边缘照明宽度
    float rim = rimStrength * max(0.0, 1.0 - dot(Normal, viewDir)) / rimWidth;
    vec3 rimColor = rimlight;    // 设置边缘照明颜色

    FragColor = vec4(ambientColor + rim * rimColor, 1.0);
}


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




// #version 430 core

// uniform vec4 lightPosition;             // should be in the eye space
// uniform vec4 lightAmbient;              // light ambient color
// uniform vec4 lightDiffuse;              // light diffuse color
// uniform vec4 lightSpecular;             // light specular color
// uniform vec4 materialAmbient;           // material ambient color
// uniform vec4 materialDiffuse;           // material diffuse color
// uniform vec4 materialSpecular;          // material specular color
// uniform float materialShininess;        // material specular shininess
// uniform mat4 u_vm;

// // In
// in vec3 esVertex;
// in vec3 esNormal;

// // output
// out vec4 fragColor;

// void main()
// {
	// // vec3 n = normalize(mat3(u_vm) * esNormal);      // convert normal to view space, u_vm (view matrix), is a rigid body transform.
	// // vec3 p = vec3(u_vm * vec4(esVertex, 1.0));                   // position in view space
	// vec3 n = esNormal;
	// vec3 p = esVertex;
	// vec3 v = normalize(-p);                       // eye vector
	// float vdn = 1.0 - max(dot(v, n), 0.0);        // the rim contribution
	 
	// fragColor.a = 1.0;
	// fragColor.rgb = vec3(smoothstep(0.8, 1.0, vdn));

// }