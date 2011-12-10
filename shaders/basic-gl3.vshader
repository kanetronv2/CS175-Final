#version 130

uniform mat4 uProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;
uniform vec3 uColor;

in vec3 aPosition;
in vec3 aNormal;

out vec3 vColor;
out vec3 vNormal;
out vec4 vPosition;

void main() {
  vColor = uColor;                                              // send color to fragment shader
  vec4 normal = vec4(aNormal.x, aNormal.y, aNormal.z, 0.0);     // convert (vec3)aNormal to vec4
  vNormal = vec3(uNormalMatrix * normal);                       // send new normal to fragment shader.

  // send position (eye coordinates) to fragment shader
  vPosition = uModelViewMatrix * vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
  gl_Position = uProjMatrix * vPosition;                        // output gl_Position is a vec4
}