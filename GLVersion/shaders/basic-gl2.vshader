uniform mat4 uProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;
uniform vec3 uColor;

attribute vec3 aPosition;
attribute vec3 aNormal;

varying vec3 vColor;
varying vec3 vNormal;
varying vec4 vPosition;

void main() {
  vColor = uColor;                                              // send color to fragment shader
  vec4 normal = vec4(aNormal.x, aNormal.y, aNormal.z, 0.0);     // convert (vec3)aNormal, to vec4
  vNormal = vec3(uNormalMatrix * normal);                       // send new normal to fragment shader.

  // send position (eye coordinates) to fragment shader
  vPosition = uModelViewMatrix * vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
  gl_Position = uProjMatrix * vPosition;                        // output gl_Position is a vec4
}
