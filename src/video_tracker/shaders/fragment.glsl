#version 330 core

in vec2 tex_coord;
out vec4 colour;

uniform sampler2D y_texture;
uniform sampler2D uv_texture;

void main()
{
    // expand limited range and center chroma values
    float y = (texture(y_texture, tex_coord).r * 255.0 - 16.0) / 219.0;
    vec2 cbcr = (texture(uv_texture, tex_coord).rg * 255.0 - 128.0) / 224.0;

    // BT.709 conversion.
    float r = y + 1.5748 * cbcr.y;
    float g = y - 0.187324 * cbcr.x - 0.468124 * cbcr.y;
    float b = y + 1.8556 * cbcr.x;

    colour = vec4(clamp(vec3(r, g, b), 0.0, 1.0), 1.0);
}
