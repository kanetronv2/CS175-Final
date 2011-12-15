uniform vec3 uLight;

varying vec3 vColor;

void main() {
  gl_FragColor = vec4(vColor.x, vColor.y, vColor.z, 1.0);
}
