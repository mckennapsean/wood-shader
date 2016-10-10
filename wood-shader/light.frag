// light model fragment shader

// from main program, light intensity
uniform float intensity;

void main(){
  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0) * intensity;
}
