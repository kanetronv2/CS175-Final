//
// hello.vert
//



uniform mat4 uProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;

attribute vec3 vColor;

varying vec3 pColor;
varying vec3 pNormal;
varying vec4 pPosition;



void main() {
  pColor = vColor;                                                                                                                                                      // send color to fragment shader
  vec4 normal = vec4(gl_Normal.x, gl_Normal.y, gl_Normal.z, 0.0);                                                       // convert (vec3)gl_Normal, to vec4
  pNormal = vec3(uNormalMatrix * normal);                                                                                                       // send new normal to fragment shader.

  pPosition = uModelViewMatrix * gl_Vertex;                                                                                                     // send position (eye coordinates) to fragment shader
  gl_Position = uProjMatrix * pPosition;                                                                                                        // output gl_Position is a vec4
}


