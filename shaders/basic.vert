#version 450 core

layout(location = 0) in vec3 in_pos;

uniform vec3 translate;
uniform mat4 view;

void main() {
  gl_Position = view * vec4(in_pos + translate, 1);
  gl_Position.w = gl_Position.z;
}