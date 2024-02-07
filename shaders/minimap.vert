#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform float offset_x;
uniform float offset_y;

void main()
{
    gl_Position = vec4(aPos.x - offset_x, aPos.y - offset_y, 0.0, 1.0);
    TexCoords = vec2(aTexCoords.x - offset_x, aTexCoords.y - offset_y);
}

