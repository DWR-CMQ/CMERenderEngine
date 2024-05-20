#version 430 core
uniform mat4 matrixModelView;
uniform mat4 matrixNormal;
uniform mat4 matrixModelViewProjection;
// vertex attribs (input)
layout(location=0) in vec3 vertexPosition;
layout(location=1) in vec3 vertexNormal;
// varyings (output)
out vec3 esVertex;
out vec3 esNormal;
void main()
{
    esVertex = vec3(matrixModelView * vec4(vertexPosition, 1.0));
    esNormal = vec3(matrixNormal * vec4(vertexNormal, 1.0));
    gl_Position = matrixModelViewProjection * vec4(vertexPosition, 1.0);
}
