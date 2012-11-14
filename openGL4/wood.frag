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

// grab eye vector
varying vec4 e;

// grab ambient, diffuse lighting terms
varying vec4 diff;
varying vec4 amb;

// fresnel calculation
// found CG implementation
//http://http.download.nvidia.com/developer/SDK/Individual_Samples/DEMOS/Direct3D9/src/HLSL_FresnelReflection/docs/FresnelReflection.pdf
float fresnel(vec3 light, vec3 norm, float var){
  float cosAngle = 1.0 - clamp(dot(light, norm), 0.0, 1.0);
  float partial0 = cosAngle * cosAngle;
  float partial1 = partial0 * partial0;
  float partial2 = partial1 * cosAngle;
  float partial3 = partial2 * (1.0 - clamp(var, 0.0, 1.0)) + var;
  float result = clamp(partial3, 0.0, 1.0);
  return result;
}

void main(){
  
  
  
  // default variables for wood shading model
  
  
  // renormalize for fragment
  vec3 norm = normalize(n);
  vec3 l = normalize(ldir);
  vec3 h = lhalf;
  vec4 eye = normalize(e);
  
  // index of refraction for the surface coat (finish), no coat if 0
  float eta = 1.5;
  
  // direction of the wood fibers
  vec3 fiber = vec3(1.0, 0.0, 0.0);
  
  // width of sub-surface highlight (along a cone)
  float beta = 0.1745;
  
  // directional color along the wood fiber
  vec3 fiberC = vec3(1.0, 0.6, 0.4);
  
  // roughness factor (for specular highlight)
  float roughness = 0.2;
  
  // MINV, MAXV, KA, KD, KS????
  
  // get the forward-facing normal
  vec3 forwardFacingNormal;
  if(gl_FrontFacing){
    forwardFacingNormal = norm;
  }else{
    forwardFacingNormal = -norm;
  }
  
  // define a local XYZ coordinate system
  // Z: out from surface, X: along fiber grain
  vec3 localZ = forwardFacingNormal;
  vec3 localX = -normalize(fiber);
  vec3 localY = cross(localZ, localX);
  
  // final color to output
  vec4 c = vec4(0.0, 0.0, 0.0, 0.0);
  
  // add global ambient to color
  c += amb;
  
  
  
  // begin shading model
  
  
  // calculate refraction & attenuation from the surface coat
  vec3 subSurfaceDir;
  float subSurfaceAtten;
  float subSurfaceFresnel;
  if(eta != 1.0){
    // need to implement the fresnel function
    // made a guess, not sure if correct...
    float r0 = (1.0 - eta) * (1.0 - eta) / (1.0 + eta) / (1.0 + eta);
    subSurfaceDir = -eye.xyz;
    float attFactor = fresnel(-subSurfaceDir, forwardFacingNormal, r0);
    subSurfaceAtten = 1.0 - attFactor;
  }else{
    subSurfaceDir = -eye.xyz;
    subSurfaceAtten = 1.0;
  }
  subSurfaceDir = normalize(subSurfaceDir);
  
  // load default parameters (not form texture maps yet)
  vec4 highlight;
  //diffuse = ???;
  highlight = vec4(fiberC.x, fiberC.y, fiberC.z, 1.0);
  vec3 axis;
  axis = fiber.x * localX + fiber.y * localY + fiber.z * localZ;
  axis = normalize(axis);
  
  // get the anisotropic highlight
  
  // refract at the smooth surface
  vec3 subSurfaceDirIn;
  float subSurfaceAttenOut;
  if(eta != 1.0){
    float etaInv = 1.0 / eta;
    float r0 = (1.0 - etaInv) * (1.0 - etaInv) / (1.0 + etaInv) / (1.0 + etaInv);
    float attFactor = fresnel(subSurfaceDir, localZ, r0);
    subSurfaceAttenOut = 1.0 - attFactor;
  }else{
    subSurfaceDirIn = -l;
    subSurfaceAttenOut = 1.0;
  }
  
  // this is a guess, for a dot product.... not sure EXACTLY renderman works...
  
  // get a factor for rendering specular highlight
  float subSurfaceFactor = max(0, dot(-subSurfaceDirIn, localZ)) * subSurfaceAtten * subSurfaceAttenOut;
  
  // calculate the angles for incidence & reflection & for the Gaussian model
  float psiR = asin(dot(subSurfaceDir, axis));
  float psiI = asin(dot(subSurfaceDirIn, axis));
  float psiD = psiR - psiI;
  float psiH = psiR + psiI;
  
  // use a Gaussian to model the specular reflection as a cone (spread out)
  float sqrt2pi = 2.5066283;
  float fiberFactor = beta * exp(-pow(psiH / beta, 2) / 2.0) / sqrt2pi;
  
  // add a geometric factor to the fiber factor for Gaussian specular reflection spread
  float cosI = cos(psiD / 2.0);
  float geoFactor = 1.0 / pow(cosI, 2);
  fiberFactor *= geoFactor;
  
  // skip diffuse highlight for now...?
  // MISSING some weird Cl term...
  
  // add in fiber highlight (attenuated)
  c += fiberFactor * subSurfaceFactor * highlight;
  
  // calculate strength of surface highlight
  vec3 vec = normalize(l - eye.xyz);
  float etaInv = 1.0 / eta;
  float r0 = (1.0 - etaInv) * (1.0 - etaInv) / (1.0 + etaInv) / (1.0 + etaInv);
  float specFactor = fresnel(eye.xyz, vec, r0);
  c += specFactor * pow(max(0, dot(vec, localZ)), 1.0 / roughness);
  
  
  
  // back to regular Phong shading
  
  
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
