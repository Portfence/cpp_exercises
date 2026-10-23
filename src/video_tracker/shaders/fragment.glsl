#version 330 core

in vec2 uv;
out vec4 colour;

uniform sampler2D frame_texture;

void main()
{
    float y = texture(frame_texture, uv).r;
    colour = vec4(y, y, y, 1.0);
}
