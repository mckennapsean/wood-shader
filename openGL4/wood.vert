// by Sean McKenna
// followed the paper, referenced in readme

// store lighting vars
varying vec3 ldir;
varying vec3 lhalf;
varying float ldist;

// store vertex position & normal
varying vec3 p;
varying vec3 n;

// store ambient and diffuse lighting terms
varying vec4 diff;
varying vec4 amb;

void main(){
  // calculate ambient and diffuse lighting
  amb = gl_LightSource[0].ambient;
  amb += gl_LightModel.ambient;
  diff = gl_LightSource[0].diffuse;
  
  // vertex position
  p = gl_Vertex.xyz;
  
  // normal for vertex, in eye coords
  n = normalize(gl_NormalMatrix * gl_Normal);
  
  // light position for vertex
  vec4 vPos = gl_ModelViewMatrix * gl_Vertex;
  vec3 aux = vec3(gl_LightSource[0].position - vPos);
  ldir = normalize(aux);
  ldist = length(aux);
  
  // calculate the light half vector
  lhalf = normalize(gl_LightSource[0].halfVector.xyz);
  
  // set vertex position
  gl_Position = ftransform();
}
