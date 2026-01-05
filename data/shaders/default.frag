//
// default.frag - Обычный шейдер пикселей.
//

#version 330 core

uniform float u_time;
uniform vec2 u_resolution;
uniform sampler2D u_texture;

// Координаты текстуры и выходной цвет:
in vec2 v_texcoord;
out vec4 FragColor;

// Основная функция:
void main() {
    vec2 uv = gl_FragCoord.xy/u_resolution.xy;
    vec4 rgba = vec4(0.5 + 0.5 * cos(u_time + uv.xyx + vec3(0, 2, 4)), 1.0);
    FragColor = rgba * texture(u_texture, v_texcoord);
}
