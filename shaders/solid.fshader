//
// newshader.frag
//



uniform vec3 uLight;

varying vec3 pColor;
varying vec3 pNormal;
varying vec4 pPosition;


void main() {
//	vec3 color = pColor + pNormal*0.0 + vec3(pPosition*0.0) + uLight*0.0;			// we use all variables so compiler doesn't optimize them away
  gl_FragColor = vec4(pColor.x, pColor.y, pColor.z, 1.0);
}
