#version 330 core

out vec2 tex_coord;

const vec2 corners[4] = vec2[4](
    vec2(-1.0, -1.0), // bottom left
    vec2( 1.0, -1.0), // bottom right
    vec2(-1.0,  1.0), // top left
    vec2( 1.0,  1.0)  // top right
);

void main()
{
    vec2 p = corners[gl_VertexID];
    gl_Position = vec4(p, 0.0, 1.0);

    tex_coord = vec2(
        (p.x + 1.0) * 0.5,
        (1.0 - p.y) * 0.5
    );
}
