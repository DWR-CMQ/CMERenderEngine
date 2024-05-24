#version 430 core

uniform vec4 lightPosition;             // should be in the eye space
uniform vec4 lightAmbient;              // light ambient color
uniform vec4 lightDiffuse;              // light diffuse color
uniform vec4 lightSpecular;             // light specular color
uniform vec4 materialAmbient;           // material ambient color
uniform vec4 materialDiffuse;           // material diffuse color
uniform vec4 materialSpecular;          // material specular color
uniform float materialShininess;        // material specular shininess

// varyings (input)
in vec3 esVertex;
in vec3 esNormal;
// output
out vec4 fragColor;
void main()
{
	vec3 n = esNormal;
	vec3 p = esVertex;
	vec3 v = normalize(-p);                       // eye vector
	float vdn = 1.0 - max(dot(v, n), 0.0);        // the rim contribution
	 
	fragColor.a = 1.0;
	fragColor.rgb = vec3(smoothstep(0.8, 1.0, vdn));
	
    // vec3 normal = normalize(esNormal);
    // vec3 light;
    // if(lightPosition.w == 0.0)
    // {
        // light = normalize(lightPosition.xyz);
    // }
    // else
    // {
        // light = normalize(lightPosition.xyz - esVertex);
    // }
    // vec3 view = normalize(-esVertex);
    // vec3 reflectVec = reflect(-light, normal);  // 2 * N * (N dot L) - L

    // vec3 color = lightAmbient.rgb * materialAmbient.rgb;        // begin with ambient
    // float dotNL = max(dot(normal, light), 0.0);
    // color += lightDiffuse.rgb * materialDiffuse.rgb * dotNL;    // add diffuse

    // float dotVR = max(dot(view, reflectVec), 0.0);
    // color += pow(dotVR, materialShininess) * lightSpecular.rgb * materialSpecular.rgb; // add specular
    // fragColor = vec4(color, materialDiffuse.a);                 // set frag color
}