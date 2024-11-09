#version 330 core

layout(location = 0) in vec2 position_in;
layout(location = 1) in vec2 textureCoords_in;

out vec2 textureCoords;

void main()
{
    textureCoords = textureCoords_in;

    gl_Position = vec4(position_in.x, position_in.y, 0.0f, 1.0f);
}
