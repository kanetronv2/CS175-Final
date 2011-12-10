#version 130

in vec3 vColor;

out vec4 fragColor;

void main() {
  fragColor = vec4(vColor.x, vColor.y, vColor.z, 1.0);
}
