#version 300 es
precision mediump float;

layout (location = 0) in vec2 vPosition;
layout (location = 1) in vec4 vColor;
layout (location = 2) in float zIndex;


uniform mat4 viewModel;
uniform mat4 projection;

out vec4 color;

void main()
{
    color = vColor;
    gl_Position = projection * viewModel * vec4(vPosition.xy, zIndex, 1.0);
    gl_PointSize=10.0;
}
