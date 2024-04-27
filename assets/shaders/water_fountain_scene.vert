#version 430 core
            
layout(location = 0) in vec3 pos;
layout(location = 1) in float life;
layout(location = 2) in vec3 delta_position;

out VS_OUT
{
    float alpha;
    vec3 delta_position;
} primitive;

uniform mat4 modelMatrix;

void main()
{
    gl_Position = modelMatrix * vec4(pos, 1.0f);

    primitive.alpha = life;
    primitive.delta_position = delta_position;
}


