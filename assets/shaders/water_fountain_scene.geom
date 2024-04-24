#version 430 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT
{
    float alpha;
    vec3 delta_position;
} primitive[];

// layout (std140, binding = 0) uniform GlobalAttributes
// {
    // mat4 view;
    // mat4 proj;
    // vec3 cam_pos;
// };
uniform mat4 view;
uniform mat4 proj;
uniform vec3 cam_pos;

out vec2 gTextureCoord;
out float gAlpha;

void main()
{
    mat4 VP = proj * view;

    vec3 u = mat3(view) * primitive[0].delta_position; // movement in view
    float w = .025f; // half width
    float h = w * 2.f;                // half height
    float t = 0;
    float nz = abs(normalize(u).z);
    if (nz > 1.f - 1e-7f)                        // the more the delta position aligns with Z axis
        t = (nz - (1.f - 1e-7f)) / 1e-7f;        // the more t will close to 1 such that h will close to w
    else if (dot(u, u) < 1e-7f)
        t = (1e-7f - dot(u,u)) / 1e-7f;
    u.z = 0.f;
    u = normalize(mix(normalize(u), vec3(1.f,0.f,0.f), t));
    h = mix(h, w, t);

    vec3 v = vec3(-u.y, u.x, 0.f);
    vec3 a = u * mat3(view);
    vec3 b = v * mat3(view);
    vec3 c = cross(a, b);
    mat3 basis = mat3(a, b, c);

    vec3 right_vec = basis * vec3(0.f, w, 0.f);
    vec3 up_vec = basis * vec3(h, 0.f, 0.f);

//               vec3 up_vec = vec3(view[0][0], view[1][0], view[2][0]) * .025f;
//               vec3 right_vec = vec3(view[0][1], view[1][1], view[2][1]) * .025f;

    gAlpha = primitive[0].alpha;

    gTextureCoord = vec2(0, 0);
    gl_Position = VP * vec4(gl_in[0].gl_Position.xyz
            + (-right_vec - up_vec), 1.f);
    EmitVertex();
    gTextureCoord = vec2(1, 0);
    gl_Position = VP * vec4(gl_in[0].gl_Position.xyz
            + (right_vec -  up_vec), 1.f);
    EmitVertex();
    gTextureCoord = vec2(0, 1);
    gl_Position = VP * vec4(gl_in[0].gl_Position.xyz
            + (-right_vec +  up_vec), 1.f);
    EmitVertex();
    gTextureCoord = vec2(1, 1);
    gl_Position = VP * vec4(gl_in[0].gl_Position.xyz
            + (right_vec +  up_vec), 1.f);
    EmitVertex();

    EndPrimitive();
}