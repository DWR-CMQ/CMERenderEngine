#version 460 core
layout(location = 0) in vec3 vertexPos;

out vec3 skyboxCoords;

uniform mat4 view;
uniform mat4 projection;

void main() 
{
  // No model transform needed for a skybox.
  vec4 pos = projection * view * vec4(vertexPos, 1.0);
  gl_Position = pos.xyww;
  // The sample coordinates are equivalent to the interpolated vertex positions.
  skyboxCoords = vertexPos;
}