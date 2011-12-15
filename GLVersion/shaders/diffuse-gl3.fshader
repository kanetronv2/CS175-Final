#version 130

uniform vec3 uLight, uLight2;

in vec3 vColor;
in vec3 vNormal;
in vec4 vPosition;

out vec4 fragColor;

void main() {
  vec3 tolight = normalize(uLight - vec3(vPosition));
  vec3 tolight2 = normalize(uLight2 - vec3(vPosition));
  vec3 normal = normalize(vNormal);             // we normalize per-fragment

  float diffuse = max(0.0, dot(normal, tolight));       // constants are written "0.0", not "0", nor "0.", nor "0.f", nor "0.lf"
  diffuse += max(0.0, dot(normal, tolight2));
  vec3 intensity = vColor * diffuse;

  fragColor = vec4(intensity.x, intensity.y, intensity.z, 1.0);       // output fragColor is a vec4 holding the pixel color for this fragment
}
