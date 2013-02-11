// Copyright 2013 Sean McKenna
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

// vertex shader that renders the shading model for wood

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
