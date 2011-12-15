//
// hello.frag
//


uniform vec3 uLight;

varying vec3 pColor;                                                                                                                                            // these variables are set by the vertex shader
varying vec3 pNormal;
varying vec4 pPosition;



void main() {
  vec3 tolight = normalize(uLight - vec3(pPosition));
  vec3 normal = normalize(pNormal);                                                                                                                     // we normalize per-fragment

  float diffuse = max(0.0, dot(normal, tolight));                                                                                       // constants are written "0.0", not "0", nor "0.", nor "0.f", nor "0.lf"
  vec3 intensity = pColor * diffuse;

  gl_FragColor = vec4(intensity.x, intensity.y, intensity.z, 1.0);                                                      // output gl_FragColor is a vec4 holding the pixel color for this fragment
}
