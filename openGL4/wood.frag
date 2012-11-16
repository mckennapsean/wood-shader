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
varying vec4 spec;

// from main program, light intensity
uniform float intensity;

// fresnel calculation
// found CG implementation
//http://http.download.nvidia.com/developer/SDK/Individual_Samples/DEMOS/Direct3D9/src/HLSL_FresnelReflection/docs/FresnelReflection.pdf
float fresnel(vec3 light, vec3 norm, float var){
  float cosAngle = 1.0 - clamp(dot(light, norm), 0.0, 1.0);
  float result = cosAngle * cosAngle;
  result *= result;
  result *= cosAngle;
  result *= (1.0 - clamp(var, 0.0, 1.0)) + var;
  result = clamp(result, 0.0, 1.0);
  return result;
}

void main(){
  
  
  
  // default variables for wood shading model
  
  
  // renormalize for fragment
  vec3 norm = normalize(n);
  vec3 l = normalize(ldir);
  vec3 h = normalize(lhalf);
  vec4 eye = normalize(e);
  
  // index of refraction for the surface coat (finish), no coat if 0
  float eta = 1.5;
  
  // direction of the wood fibers
  vec3 fiber = vec3(1.0, 0.0, 0.0);
  
  // width of sub-surface highlight (along a cone)
  float beta = 0.1745;
  
  // roughness factor (for specular highlight, originally 0.2)
  float roughness = 0.03;
  
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
  vec3 localY = cross(-localZ, localX);
  
  // final color to output
  vec4 c = vec4(0.0, 0.0, 0.0, 0.0);
  
  // add global ambient to color
  c += amb;
  
  
  
  // begin shading model
  
  
  // calculate refraction & attenuation from the surface coat
  vec3 subSurfaceDir = -normalize(eye.xyz);
  float subSurfaceAtten;
  if(eta != 1.0){
    // fresnel function, implementation above
    float r0 = (1.0 - eta) * (1.0 - eta) / (1.0 + eta) / (1.0 + eta);
    float attFactor = fresnel(-subSurfaceDir, forwardFacingNormal, r0);
    subSurfaceAtten = 1.0 - attFactor;
  }else{
    subSurfaceAtten = 1.0;
  }
  
  // load default parameters (not from texture maps yet)
  // colors from GL material
  vec3 axis;
  axis = fiber.x * localX + fiber.y * localY + fiber.z * localZ;
  axis = normalize(axis);
  
  // get the anisotropic highlight
  
  // refract at the smooth surface
  vec3 subSurfaceDirIn = -l;
  float subSurfaceAttenIn;
  if(eta != 1.0){
    float etaInv = 1.0 / eta;
    float r0 = (1.0 - etaInv) * (1.0 - etaInv) / (1.0 + etaInv) / (1.0 + etaInv);
    float attFactor = fresnel(subSurfaceDirIn, localZ, r0);
    subSurfaceAttenIn = 1.0 - attFactor;
  }else{
    subSurfaceAttenIn = 1.0;
  }
  
  // get a factor for rendering specular highlight (Phong-style calculation)
  float subSurfaceFactor = max(0, dot(-subSurfaceDirIn, localZ)) * subSurfaceAtten * subSurfaceAttenIn;
  
  // calculate the angles for incidence & reflection & for the Gaussian model
  float psiR = asin(dot(subSurfaceDir, axis));
  float psiI = asin(dot(subSurfaceDirIn, axis));
  float psiD = psiR - psiI;
  float psiH = psiR + psiI;
  
  // use a Gaussian to model the specular reflection as a cone (spread out)
  float sqrt2pi = 2.5066283;
  float fiberFactorInitial = exp(-pow(psiH / beta, 2) / 2.0) / sqrt2pi / beta;
  
  // create a geometric factor to the fiber factor for Gaussian specular reflection spread
  float cosI = cos(psiD / 2.0);
  float geoFactor = 1.0 / pow(cosI, 2);
  float fiberFactor = fiberFactorInitial * geoFactor;
  
  // add in diffuse term (attenuated)
  c += subSurfaceFactor * diff * intensity;
  
  // add in fiber highlight (attenuated)
  c += fiberFactor * subSurfaceFactor * spec * intensity;
  
  // calculate strength of surface highlight
  vec3 vec = normalize(l - eye.xyz);
  float etaInv = 1.0 / eta;
  float r0 = (1.0 - etaInv) * (1.0 - etaInv) / (1.0 + etaInv) / (1.0 + etaInv);
  float specFactor = fresnel(eye.xyz, vec, r0);
  
  // add in surface highlight (Phong-style highlight)
  c += specFactor * pow(max(0, dot(vec, localZ)), 1.0 / roughness) * spec * intensity;
  
  // set the output color
  gl_FragColor = c;
}
