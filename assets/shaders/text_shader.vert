#version 330 core
layout (location = 0) in vec4 v;
uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform mat4 dc2ndc;

out vec2 t_coord;

void main()
{
   gl_Position = proj * view * model * dc2ndc * vec4(v.xy, 0.0, 1.0);
   t_coord = v.zw;
}
