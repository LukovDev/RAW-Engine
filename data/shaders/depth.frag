//
// default.frag - Обычный шейдер пикселей.
//

#version 330 core

uniform sampler2D u_texture;
uniform float far;
uniform float near;

// Координаты текстуры и выходной цвет:
in vec2 v_texcoord;
out vec4 FragColor;


// Основная функция:
void main() {
    float depth = texture(u_texture, vec2(v_texcoord.x, -v_texcoord.y)).r;
    float linearDepth = (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));

    float vis = (linearDepth - near) / (far - near);
    FragColor = vec4(vec3(vis), 1.0);
}
