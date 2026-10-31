#version 450

layout(location = 0) out vec3 outColor;
layout(push_constant) uniform Transform {
    mat4 mvp;
} transform;

void main() {
    const vec3 corners[8] = vec3[](
        vec3(-0.5, -0.5, -0.5), vec3(0.5, -0.5, -0.5),
        vec3(0.5, 0.5, -0.5), vec3(-0.5, 0.5, -0.5),
        vec3(-0.5, -0.5, 0.5), vec3(0.5, -0.5, 0.5),
        vec3(0.5, 0.5, 0.5), vec3(-0.5, 0.5, 0.5)
    );
    const int indices[36] = int[](
        0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7,
        0, 4, 7, 0, 7, 3, 1, 2, 6, 1, 6, 5,
        0, 1, 5, 0, 5, 4, 3, 7, 6, 3, 6, 2
    );
    vec3 position = corners[indices[gl_VertexIndex]];
    gl_Position = transform.mvp * vec4(position, 1.0);
    outColor = position + vec3(0.5);
}
