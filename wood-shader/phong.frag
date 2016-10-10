// Phong fragment shader

// grab lighting vars
varying vec3 ldir;
varying vec3 lhalf;
varying float ldist;

// grab the vertex normal
varying vec3 n;

// grab ambient, diffuse lighting terms
varying vec4 diff;
varying vec4 amb;

void main(){
  // final color to output
  vec4 c;
  
  // set color to the global ambient
  c = amb;
  
  // renormalize for fragment
  vec3 norm = normalize(n);
  vec3 l = normalize(ldir);
  vec3 h = lhalf;
  
  // sets the diffuse darkness (dot product betwee normal and light)
  float dotProd = max(dot(norm, l), 0.0);

  // add diffuse & specular highlights if bright
  if(dotProd > 0.0){
    // calculate attenuation
    float att = 1.0 / (gl_LightSource[0].constantAttenuation + gl_LightSource[0].linearAttenuation * ldist + gl_LightSource[0].quadraticAttenuation * ldist * ldist);
    
    // calculate diffuse and specular brightness
    c += att * (diff * dotProd);
    c += att * gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(max(dot(norm, h), 0.0), gl_FrontMaterial.shininess);
  }
  
  // set the output color
  gl_FragColor = c;
}
