// by Sean McKenna
// followed the paper, referenced in readme

// store lighting vars
varying vec3 ldir;
varying vec3 lhalf;
varying float ldist;

// store vertex position & normal
varying vec3 p;
varying vec3 n;

// store eye vector
varying vec4 e;

// store ambient and diffuse and specular lighting terms
varying vec4 diff;
varying vec4 amb;
varying vec4 spec;

void main(){
  // set the texture coordinate (only need one)
  gl_TexCoord[0] = gl_MultiTexCoord1;
  
  // calculate ambient and diffuse lighting
  amb = gl_LightSource[0].ambient + gl_LightModel.ambient;
  diff = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
  spec = gl_LightSource[0].specular;
  
  // vertex position
  p = gl_Vertex.xyz;
  
  // normal for vertex, in eye coords
  n = normalize(gl_NormalMatrix * gl_Normal);
  
  // eye position
  vec4 vPos = gl_ModelViewMatrix * gl_Vertex;
  e = normalize(vPos);
  
  // light position
  vec3 aux = vec3(gl_LightSource[0].position - vPos);
  ldir = normalize(aux);
  ldist = length(aux);
  
  // calculate the light half vector
  lhalf = normalize(gl_LightSource[0].halfVector.xyz);
  
  // set vertex position
  gl_Position = ftransform();
}
