#version 430 core

// In
in vec3 fragPos;

// Uniform
uniform vec3 color;

// Out
out vec4 fragColor;

void main()
{
	vec3 temp = mix(fragPos / 2.0 + 0.5, color, 0.5);
    fragColor = vec4(temp, 1.0); 
}