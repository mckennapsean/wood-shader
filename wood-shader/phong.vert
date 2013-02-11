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

// Phong vertex shader

// store lighting vars
varying vec3 ldir;
varying vec3 lhalf;
varying float ldist;

// store vertex normal
varying vec3 n;

// store ambient and diffuse lighting terms
varying vec4 diff;
varying vec4 amb;

void main(){
  // calculate ambient and diffuse lighting
  amb = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
  amb += gl_LightModel.ambient * gl_FrontMaterial.ambient;
  diff = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
  
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
