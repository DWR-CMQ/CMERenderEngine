#version 430 core
in float gAlpha;
in vec2 gTextureCoord;

uniform sampler2D sprite;
uniform vec3 baseColor;
out vec4 color;

void main()
{
   color = vec4(baseColor, gAlpha * texture(sprite, gTextureCoord).r);
}

