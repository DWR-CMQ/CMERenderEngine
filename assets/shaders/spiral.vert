#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 v_normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
// uniform vec3 lightDirection;
// vec3 lightDirection = vec3(0.0f, 0.0f, -10.0f);
out vec3 FragPos;
out vec3 Normal;
void main()
{
	FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * v_normal;

	gl_Position = projection * view * vec4(FragPos, 1.0);
}


// #version 430 core
// layout (location = 0) in vec3 position;
// layout (location = 1) in vec3 v_normal;

// uniform mat4 projection;
// uniform mat4 view;
// uniform mat4 model;
// //uniform vec3 lightDirection;
// vec3 lightDirection = vec3(0.0f, 0.0f, -10.0f);
// out VS_OUT 
// {
    // vec3 N;
    // vec3 L;
    // vec3 V;
// } vs_out;

// void main()
// {
	// vec3 P = vec3(model * vec4(position, 1.0));
    // vs_out.N = mat3(transpose(inverse(model))) * v_normal;
    // vs_out.L = lightDirection - P.xyz;
    // vs_out.V = -P.xyz;
	// gl_Position = projection * view * vec4(P, 1.0);

// }

// #version 430 core

// uniform mat4 matrixModelViewProjection;
// uniform mat4 matrixModelView;
// uniform mat4 matrixNormal;

// // In
// layout(location=0) in vec3 vertexPosition;
// layout(location=1) in vec3 vertexNormal;

// // Out
// // out vec3 fragPos;
// // out vec3 fragPos;
// out vec3 esVertex;
// out vec3 esNormal;

// void main()
// {
	// //fragPos = vertexPosition;
	// esVertex = vec3(matrixModelView * vec4(vertexPosition, 1.0));
	// // esNormal = vec3(matrixNormal * vec4(vertexNormal, 1.0));
	// esNormal = vertexNormal;
    // gl_Position = matrixModelViewProjection * vec4(vertexPosition, 1.0);
// }
