#version 430 core
uniform mat4 matrixModelViewProjection;
// vertex attribs (input)
layout(location=0) in vec3 vertexPosition;
// layout(location=1) in vec3 vertexNormal;

out vec3 fragPos;

void main()
{
	fragPos = vertexPosition;
    gl_Position = matrixModelViewProjection * vec4(vertexPosition, 1.0);
}
