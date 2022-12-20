#version 300 es
precision mediump float;
layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 aTexCoord;
//attribute vec2 aTexCoord;
uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;
out vec2 TexCoord;

void main(void) {
    //gl_Position = Position;
    gl_Position = Projection * View * Model * vec4(Position,0.0,1.0);
    TexCoord = aTexCoord;
}