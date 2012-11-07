// by Sean McKenna
// followed the paper, referenced in readme
// reference for fresnel function: http://code.google.com/p/glslang-library/ ???

// grab lighting vars
varying vec3 ldir;
varying vec3 lhalf;
varying float ldist;

// grab the vertex position & normal
varying vec3 p;
varying vec3 n;

// grab ambient, diffuse lighting terms
varying vec4 diff;
varying vec4 amb;

// fresnel calculation
// found CG implementation
//http://http.download.nvidia.com/developer/SDK/Individual_Samples/DEMOS/Direct3D9/src/HLSL_FresnelReflection/docs/FresnelReflection.pdf
float fresnel(vec3 light, vec3 norm, float var){
  float cosAngle = 1 - clamp(dot(light, norm), 0.0, 1.0);
  float result = cosAngle * cosAngle;
  result = result * result;
  result = result * cosAngle;
  result = clamp(result * (1 - clamp(var, 0.0, 1.0)) + var, 0.0, 1.0);
  return result;
}

void main(){
  
  
  
  // default variables for wood shading model
  
  
  // index of refraction for the surface coat (finish), no coat if 0
  float eta = 1.5;
  
  // direction of the wood fibers
  vec3 fiber = vec3(1.0, 0.0, 0.0);
  
  // width of sub-surface highlight (along a cone)
  float beta = 0.1745;
  
  // directional color along the wood fiber
  vec3 fiberC = vec3(1.0, 0.6, 0.4);
  
  // MINV, MAXV, KA, KD, KS, ROUGHNESS???
  
  // final color to output
  vec4 c;
  
  
  
  // begin shading model
  
  
  // calculate refraction & attenuation from the surface coat
  if(eta != 1.0){
    // need to implement the fresnel function, not sure how...
    // check above
    subSurfaceDir = -normalize(ldir);
    subSurfaceAtt = 1.0;
  }else{
    subSurfaceDir = -normalize(ldir);
    subSurfaceAtt = 1.0;
  }
  normalize(subSurfaceDir);
  
  
  
  
  
  
  
  
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
