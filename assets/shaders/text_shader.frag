#version 330 core
in vec2 t_coord;
out vec4 color;

uniform sampler2D text;
uniform vec4 text_color;

void main()
{
color = vec4(text_color.xyz, texture(text, t_coord).r*text_color.w);
}
