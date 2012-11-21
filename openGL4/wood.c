// by Sean McKenna for CS6610 
// template from getting_started example
//
// renders a wood shading model using shaders

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include "GL/glew.h"
#include "GL/glut.h"
#include "GL/glui.h"
#include "GL/glu.h"
#include "lib/texture.h"
#include "lib/texture.c"

using namespace std;

int	main_window;

// shader variables
GLuint v0, f0, p0;
GLuint v1, f1, p1;
GLuint v2, f2, p2;
GLuint v3, f3, p3;
GLuint p1t, p2t, p2t2, p2t3, p2t4, p3t;

// swap between wood and non-wood shader
bool wood = true;

// whether to debug shaders or not
bool debug = true;

// track wood object rotation
double rotation = 0.0;

// store which wood type to load
// 1 for maple, 4 for paduak, 7 for walnut1, 10 for walnut2, 0 for all
int woodType = 1;

// camera info
float eye[3];
float lookat[3];

// pointers for all of the object controls
GLUI *glui;
GLUI_Rollout		*object_rollout;
GLUI_Rotation		*object_rotation;
GLUI_Translation	*object_xz_trans;
GLUI_Translation	*object_y_trans;

// light position controls
GLUI_Rollout     *light_rollout;
GLUI_Translation *light_xz_trans;
GLUI_Translation *light_y_trans;

// disable / enable features
GLUI_Checkbox *draw_floor_check;
GLUI_Checkbox *draw_walls_check;
GLUI_Checkbox *draw_object_check;

// the user id's that we can use to identify control callbacks
#define CB_DEPTH_BUFFER 0

// live variables
// each of these are associated with a control in the interface
// when the control is modified, these variables are automatically updated
float live_object_rotation[16];
float live_object_xz_trans[2];
float live_object_y_trans;
float live_light_xz_trans[2];
float live_light_y_trans;
float live_light_intensity;

int live_draw_floor;
int live_draw_walls;
int live_draw_object;

// normalize a three-dimensional vector
void normalize(float v[3]){ 
  float l = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
  l = 1 / (float)sqrt(l);
  
  v[0] *= l;
  v[1] *= l;
  v[2] *= l;
}

// dot-product of two three-dimensional vectors into a scalar
float dotproduct(float a[3], float b[3]){
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// cross-product of two three-dimensional vectors into a third vector
void crossproduct(float a[3], float b[3], float res[3]){ 
  res[0] = (a[1] * b[2] - a[2] * b[1]);
  res[1] = (a[2] * b[0] - a[0] * b[2]);
  res[2] = (a[0] * b[1] - a[1] * b[0]);
}

// length of a three-dimensional vector
float length(float v[3]){
  return (float)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

// when program is idle
void myGlutIdle(void){
  // make sure the main window is active 
  if(glutGetWindow() != main_window)
    glutSetWindow(main_window);
  
  // if you have moving objects, you can do that here
  // just keep redrawing the scene over and over
  glutPostRedisplay();
}

// mouse handling functions for the main window
// left mouse translates, middle zooms, right rotates
// keep track of which button is down and where the last position was
int cur_button = -1;
int last_x;
int last_y;

// when a mouse action is performed
void myGlutMouse(int button, int state, int x, int y){
  if (state == GLUT_DOWN)
    cur_button = button;
	else{
    if(button == cur_button)
      cur_button = -1;
  }
  last_x = x; 
  last_y = y; 
}

// when the mouse is moving around
void myGlutMotion(int x, int y){
  // the change in mouse position
  int dx = x-last_x;
  int dy = y-last_y;
  
  float scale, len, theta;
  float neye[3], neye2[3];
  float f[3], r[3], u[3];
  
  switch(cur_button){
  case GLUT_LEFT_BUTTON:
    // translate
    f[0] = lookat[0] - eye[0];
    f[1] = lookat[1] - eye[1];
    f[2] = lookat[2] - eye[2];
    u[0] = 0;
    u[1] = 1;
    u[2] = 0;
    
    // scale the change by how far away we are 
    scale = sqrt(length(f)) * 0.007;
    
    crossproduct(f, u, r);
    crossproduct(r, f, u);
    normalize(r);
    normalize(u);
    
    eye[0] += -r[0]*dx*scale + u[0]*dy*scale;
    eye[1] += -r[1]*dx*scale + u[1]*dy*scale;
    eye[2] += -r[2]*dx*scale + u[2]*dy*scale;
    
    lookat[0] += -r[0]*dx*scale + u[0]*dy*scale;
    lookat[1] += -r[1]*dx*scale + u[1]*dy*scale;
    lookat[2] += -r[2]*dx*scale + u[2]*dy*scale;
    
    break;
  
  case GLUT_MIDDLE_BUTTON:
    // zoom
    f[0] = lookat[0] - eye[0];
    f[1] = lookat[1] - eye[1];
    f[2] = lookat[2] - eye[2];
    
    len = length(f);
    normalize(f);
    
    // scale the change by how far away we are 
    len -= sqrt(len)*dx*0.03;
    
    eye[0] = lookat[0] - len*f[0];
    eye[1] = lookat[1] - len*f[1];
    eye[2] = lookat[2] - len*f[2];
    
    // make sure the eye and lookat points are sufficiently far away
    // push the lookat point forward if it is too close
    if(len < 1){
      printf("lookat move: %f\n", len);
      lookat[0] = eye[0] + f[0];
      lookat[1] = eye[1] + f[1];
      lookat[2] = eye[2] + f[2];
    }
    break;
  
  case GLUT_RIGHT_BUTTON:
    // rotate
    neye[0] = eye[0] - lookat[0];
    neye[1] = eye[1] - lookat[1];
    neye[2] = eye[2] - lookat[2];
    
    // first rotate in the x/z plane
    theta = -dx * 0.007;
    neye2[0] = (float)cos(theta)*neye[0] + (float)sin(theta)*neye[2];
    neye2[1] = neye[1];
    neye2[2] =-(float)sin(theta)*neye[0] + (float)cos(theta)*neye[2];
    
    // now rotate vertically
    theta = -dy * 0.007;
    
    f[0] = -neye2[0];
    f[1] = -neye2[1];
    f[2] = -neye2[2];
    u[0] = 0;
    u[1] = 1;
    u[2] = 0;
    crossproduct(f, u, r);
    crossproduct(r, f, u);
    len = length(f);
    normalize(f);
    normalize(u);
    
    neye[0] = len * ((float)cos(theta)*f[0] + (float)sin(theta)*u[0]);
    neye[1] = len * ((float)cos(theta)*f[1] + (float)sin(theta)*u[1]);
    neye[2] = len * ((float)cos(theta)*f[2] + (float)sin(theta)*u[2]);
    
    eye[0] = lookat[0] - neye[0];
    eye[1] = lookat[1] - neye[1];
    eye[2] = lookat[2] - neye[2];
    
    break;
  }
  
  last_x = x;
  last_y = y;
  glutPostRedisplay();
}

// when a keyboard action is performed
void myGlutKeyboard(unsigned char key, int x, int y){
  switch(key){
    
    // toggle wood shader
    case 'w':
    case 'W':
      if(wood){
        wood = false;
        cout<<"Wood shader deactivated"<<endl;
      }else{
        wood = true;
        cout<<"Wood shader activated"<<endl;
      }
      break;
    
  // quit
  case 27:
  case 'q':
  case 'Q':
    exit(0);
    break;
  }
  glutPostRedisplay();
}

// when the window is resized
void myGlutReshape(int x, int y){
  int tx, ty, tw, th;
  GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
  glViewport(tx, ty, tw, th);
  glutPostRedisplay();
}

char* readShader(const char *filename){
  // try to open shader file contents
  FILE *file = fopen(filename, "r");
  char *contents;
  int count = 0;
  
  // if missing file
  if(!file){
    fprintf(stderr, "Unable to open %s\n", filename);
    return NULL;
  }
  
  // prepare content to be read
  fseek(file, 0, SEEK_END);
  count = ftell(file);
  rewind(file);
  
  // read content into memory
  if(count > 0){
    contents = (char *) malloc(sizeof(char) * (count + 1));
    count = fread(contents, sizeof(char), count, file);
    contents[count] = '\0';
  }
  
  // clean-up
  fclose(file);
  return contents;
}

// create all shader programs
void createShaders(){
  // info log variables
  GLint Result = GL_FALSE;
  int InfoLogLength;
  
  //
  // p0 - phong shader
  //
  
  // store file contents
  char *vs0, *fs0;
  
  // initialize shaders
  v0 = glCreateShader(GL_VERTEX_SHADER);
  f0 = glCreateShader(GL_FRAGMENT_SHADER);
  
  // load shaders from file
  vs0 = readShader("phong.vert");
  fs0 = readShader("phong.frag");
  const char * vv0 = vs0;
  const char * ff0 = fs0;
  glShaderSource(v0, 1, &vv0, NULL);
  glShaderSource(f0, 1, &ff0, NULL);
  free(vs0);
  free(fs0);
  
  // compile shaders & log errors
  glCompileShader(v0);
  glGetShaderiv(v0, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(v0, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage0(InfoLogLength);
  glGetShaderInfoLog(v0, InfoLogLength, NULL, &VertexShaderErrorMessage0[0]);
  if(debug)
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage0[0]);
  glCompileShader(f0);
  glGetShaderiv(f0, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(f0, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage0(InfoLogLength);
  glGetShaderInfoLog(f0, InfoLogLength, NULL, &FragmentShaderErrorMessage0[0]);
  if(debug)
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage0[0]);
  
  // create shader programs & log errors
  p0 = glCreateProgram();
  glAttachShader(p0, v0);
  glAttachShader(p0, f0);
  glLinkProgram(p0);
  glGetProgramiv(p0, GL_LINK_STATUS, &Result);
  glGetProgramiv(p0, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage0(max(InfoLogLength, int(1)));
  if(debug)
    fprintf(stdout, "%s\n", &ProgramErrorMessage0[0]);
  
  // clear shaders
  glDeleteShader(v0);
  glDeleteShader(f0);
  
  //
  // p1 - texture shader (phong shading)
  //
  
  // store file contents
  char *vs1, *fs1;
  
  // initialize shaders
  v1 = glCreateShader(GL_VERTEX_SHADER);
  f1 = glCreateShader(GL_FRAGMENT_SHADER);
  
  // load shaders from file
  vs1 = readShader("texture.vert");
  fs1 = readShader("texture.frag");
  const char * vv1 = vs1;
  const char * ff1 = fs1;
  glShaderSource(v1, 1, &vv1, NULL);
  glShaderSource(f1, 1, &ff1, NULL);
  free(vs1);
  free(fs1);
  
  // compile shaders & log errors
  glCompileShader(v1);
  glGetShaderiv(v1, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(v1, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage1(InfoLogLength);
  glGetShaderInfoLog(v1, InfoLogLength, NULL, &VertexShaderErrorMessage1[0]);
  if(debug)
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage1[0]);
  glCompileShader(f1);
  glGetShaderiv(f1, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(f1, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage1(InfoLogLength);
  glGetShaderInfoLog(f1, InfoLogLength, NULL, &FragmentShaderErrorMessage1[0]);
  if(debug)
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage1[0]);
  
  // create shader programs & log erfrors
  p1 = glCreateProgram();
  glAttachShader(p1, v1);
  glAttachShader(p1, f1);
  glLinkProgram(p1);
  glGetProgramiv(p1, GL_LINK_STATUS, &Result);
  glGetProgramiv(p1, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage1(max(InfoLogLength, int(1)));
  if(debug)
    fprintf(stdout, "%s\n", &ProgramErrorMessage1[0]);
  
  // add program variable
  p1t = glGetUniformLocation(p1, "tex");
  
  // clear shaders
  glDeleteShader(v1);
  glDeleteShader(f1);
  
  //
  // p2 - wood shader (surface phong shading)
  //
  
  // store file contents
  char *vs2, *fs2;
  
  // initialize shaders
  v2 = glCreateShader(GL_VERTEX_SHADER);
  f2 = glCreateShader(GL_FRAGMENT_SHADER);
  
  // load shaders from file
  vs2 = readShader("wood.vert");
  fs2 = readShader("wood.frag");
  const char * vv2 = vs2;
  const char * ff2 = fs2;
  glShaderSource(v2, 1, &vv2, NULL);
  glShaderSource(f2, 1, &ff2, NULL);
  free(vs2);
  free(fs2);
  
  // compile shaders & log errors
  glCompileShader(v2);
  glGetShaderiv(v2, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(v2, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage2(InfoLogLength);
  glGetShaderInfoLog(v2, InfoLogLength, NULL, &VertexShaderErrorMessage2[0]);
  if(debug)
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage2[0]);
  glCompileShader(f2);
  glGetShaderiv(f2, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(f2, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage2(InfoLogLength);
  glGetShaderInfoLog(f2, InfoLogLength, NULL, &FragmentShaderErrorMessage2[0]);
  if(debug)
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage2[0]);
  
  // create shader programs & log erfrors
  p2 = glCreateProgram();
  glAttachShader(p2, v2);
  glAttachShader(p2, f2);
  glLinkProgram(p2);
  glGetProgramiv(p2, GL_LINK_STATUS, &Result);
  glGetProgramiv(p2, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage2(max(InfoLogLength, int(1)));
  if(debug)
    fprintf(stdout, "%s\n", &ProgramErrorMessage2[0]);
  
  // add program variable
  p2t = glGetUniformLocation(p2, "intensity");
  p2t2 = glGetUniformLocation(p2, "tex");
  p2t3 = glGetUniformLocation(p2, "texHighlight");
  p2t4 = glGetUniformLocation(p2, "texFiber");
  
  // clear shaders
  glDeleteShader(v2);
  glDeleteShader(f2);
  
  //
  // p3 - light model shader
  //
  
  // store file contents
  char *vs3, *fs3;
  
  // initialize shaders
  v3 = glCreateShader(GL_VERTEX_SHADER);
  f3 = glCreateShader(GL_FRAGMENT_SHADER);
  
  // load shaders from file
  vs3 = readShader("light.vert");
  fs3 = readShader("light.frag");
  const char * vv3 = vs3;
  const char * ff3 = fs3;
  glShaderSource(v3, 1, &vv3, NULL);
  glShaderSource(f3, 1, &ff3, NULL);
  free(vs3);
  free(fs3);
  
  // compile shaders & log errors
  glCompileShader(v3);
  glGetShaderiv(v3, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(v3, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage3(InfoLogLength);
  glGetShaderInfoLog(v3, InfoLogLength, NULL, &VertexShaderErrorMessage3[0]);
  fprintf(stdout, "%s\n", &VertexShaderErrorMessage3[0]);
  glCompileShader(f3);
  glGetShaderiv(f3, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(f3, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage3(InfoLogLength);
  glGetShaderInfoLog(f3, InfoLogLength, NULL, &FragmentShaderErrorMessage3[0]);
  fprintf(stdout, "%s\n", &FragmentShaderErrorMessage3[0]);
  
  // create shader programs & log errors
  p3 = glCreateProgram();
  glAttachShader(p3, v3);
  glAttachShader(p3, f3);
  glLinkProgram(p3);
  glGetProgramiv(p3, GL_LINK_STATUS, &Result);
  glGetProgramiv(p3, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage3(max(InfoLogLength, int(1)));
  fprintf(stdout, "%s\n", &ProgramErrorMessage3[0]);
  
  // add program variable
  p3t = glGetUniformLocation(p3, "intensity");
  
  // clear shaders
  glDeleteShader(v3);
  glDeleteShader(f3);
}

// set up the scene lighting
void lightScene(void){
  // lighting: white color, position, & direction
  GLfloat lcol[3] = {live_light_intensity, live_light_intensity, live_light_intensity};
  GLfloat lamb[3] = {0.10, 0.10, 0.10};
  GLfloat lpos[4] = {live_light_xz_trans[0], live_light_y_trans, -live_light_xz_trans[1], 0.0};
  GLfloat ldir[3] = {0.0, 0.0, 0.0};
  
  // model light as an emissive sphere
  glPushMatrix();
  glTranslatef(lpos[0], lpos[1], lpos[2]);
  glColor3f(0.0, 0.0, 0.0);
  GLfloat emission[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat noEmission[] = {0.0, 0.0, 0.0, 1.0};
  glMaterialfv(GL_FRONT, GL_EMISSION, emission);
  glUseProgram(p3);
  glUniform1f(p3t, live_light_intensity);
  glutSolidSphere(0.3, 64, 64);
  glUseProgram(p0);
  glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
  glPopMatrix();
  
  // light setup
  glLighti(GL_LIGHT0, GL_SPOT_CUTOFF, 180);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lcol);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lamb);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lamb);
}

// draw the objects in the scene
void drawObjects(){
  // draw wood block
  if(live_draw_object){
    // get object position
    glPushMatrix();
    glTranslatef(live_object_xz_trans[0], live_object_y_trans, -live_object_xz_trans[1]);
    glMultMatrixf(live_object_rotation);
    
    // rotate around the y-axis & increment appropriately
    glRotatef(rotation, 0.0, -1.0, 0.0);
    rotation += 0.4;
    if(rotation >= 360.0)
      rotation -= 360.0;
    
    // background wood color
    GLfloat wood_amb[] = {0.27969, 0.14375, 0.00250, 1.0};
    GLfloat wood_diff[] = {0.4375, 0.21875, 0.00391, 1.0};
    GLfloat wood_spec[] = {0.9975, 0.68875, 0.12391, 1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, wood_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, wood_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, wood_spec);
    
    // set shader for wood, if active
    if(wood){
      glUseProgram(p2);
      glUniform1f(p2t, live_light_intensity);
      glUniform1i(p2t2, woodType);
      glUniform1i(p2t3, woodType + 1);
      glUniform1i(p2t4, woodType + 2);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, woodType);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, woodType + 1);
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, woodType + 2);
    }else{
      glUseProgram(p0);
    }
    
    // draw wooden thin slab
    glTranslatef(0, 0.2, 0);
    glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0.0, -1.0, 0.0);
      glMultiTexCoord2fARB(GL_TEXTURE1, 0.0, 0.0);
      glVertex3f(-7, 0, -7);
      glMultiTexCoord2fARB(GL_TEXTURE1, 1.0, 0.0);
      glVertex3f( 7, 0, -7);
      glMultiTexCoord2fARB(GL_TEXTURE1, 1.0, 1.0);
      glVertex3f( 7, 0, 7);
      glMultiTexCoord2fARB(GL_TEXTURE1, 0.0, 1.0);
      glVertex3f(-7, 0, 7);
    glEnd();
    
    // clean-up
    glUseProgram(p0);
    glPopMatrix();
  }
}

// draw the floor
void drawFloor(){
  // activate texturing
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  // make floor with white base
  GLfloat white_amb[] = {1.00, 1.00, 1.00, 1.0};
  GLfloat white_diff[] = {0.50, 0.50, 0.50, 1.0};
  GLfloat white_spec[] = {0.75, 0.75, 0.75, 1.0};
  GLfloat white_shin[] = {70.0};
  glMaterialfv(GL_FRONT, GL_AMBIENT, white_amb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, white_diff);
  glMaterialfv(GL_FRONT, GL_SPECULAR, white_spec);
  glMaterialfv(GL_FRONT, GL_SHININESS, white_shin);
  
  // activate texture shader
  glUseProgram(p1);
  glUniform1i(p1t, 0);
  
  // draw the floor with textures (mip-mapped checkerboard)
  glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0, 1.0, 0.0);
    glMultiTexCoord2fARB(GL_TEXTURE0, 0.0, 0.0);
    glVertex3f(-10, 0, -10);
    glMultiTexCoord2fARB(GL_TEXTURE0, 0.0, 1.5);
    glVertex3f( 10, 0, -10);
    glMultiTexCoord2fARB(GL_TEXTURE0, 1.5, 1.5);
    glVertex3f( 10, 0, 10);
    glMultiTexCoord2fARB(GL_TEXTURE0, 1.5, 0.0);
    glVertex3f(-10, 0, 10);
  glEnd();
  
  // clean-up
  glUseProgram(p0);
}

// draw the walls with shadows
void drawWalls(void){
  // activate culling for all walls
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  
  // make walls light-blue
  GLfloat blue_amb[] = {0.13, 0.11, 0.23, 1.0};
  GLfloat blue_diff[] = {0.24, 0.23, 0.49, 1.0};
  GLfloat blue_spec[] = {0.33, 0.49, 1.00, 1.0};
  GLfloat blue_shin[] = {30.0};
  glMaterialfv(GL_FRONT, GL_AMBIENT, blue_amb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, blue_diff);
  glMaterialfv(GL_FRONT, GL_SPECULAR, blue_spec);
  glMaterialfv(GL_FRONT, GL_SHININESS, blue_shin);
  
  glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(-10, 0, -10);
    glVertex3f(-10, 20, -10);
    glVertex3f(10, 20, -10);
    glVertex3f(10, 0, -10);
  glEnd();
  
  // draw closest wall
  glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(10, 0, 10);
    glVertex3f(10, 20, 10);
    glVertex3f(-10, 20, 10);
    glVertex3f(-10, 0, 10);
  glEnd();
  
  // draw left side wall
  glBegin(GL_QUADS);
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(-10, 0, 10);
    glVertex3f(-10, 20, 10);
    glVertex3f(-10, 20, -10);
    glVertex3f(-10, 0, -10);
  glEnd();
  
  // draw right side wall
  glBegin(GL_QUADS);
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(10, 0, -10);
    glVertex3f(10, 20, -10);
    glVertex3f(10, 20, 10);
    glVertex3f(10, 0, 10);
  glEnd();
  
  // deactivate culling
  glDisable(GL_CULL_FACE);
}

// drawing the scene
void myGlutDisplay(void){
  // clear screen
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // projection transform
  glMatrixMode(GL_PROJECTION);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glFrustum(-1, 1, -1, 1, 1, 1000);
  myGlutReshape(1000, 600);
  
  // camera transform
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(eye[0], eye[1], eye[2], lookat[0], lookat[1], lookat[2], 0, 1, 0);
  
  // clear what's left
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // activate the base shader
  glUseProgram(p0);
  
  // start drawing the scene
  lightScene();
  
  // drawing the walls
  if(live_draw_walls)
    drawWalls();
  
  // drawing the floor
  if(live_draw_floor)
    drawFloor();
  
  // draw the objects (wood)
  drawObjects();
  
  // update the canvas with the next view
  glutSwapBuffers();
}

// callback for certain GLUI controls
void glui_cb(int control){
  glutPostRedisplay();
}

// initial setup
void init(void){
  GLenum err = glewInit();
  if(GLEW_OK != err){
    //failed to initialize GLEW!
    printf("failed !!!!!!!!!!!!");
  }
  printf("%s", glewGetString(GLEW_VERSION));
  
  // texture variables
  static unsigned *image;
  static int width, height, components;
  
  // import checkerboard image for textures
  image = read_texture("tex/board.rgb", &width, &height, &components);
  glBindTexture(GL_TEXTURE_2D, 0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  // automatic mip-mapping
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid *) image);
  free(image);
  
  // import maple wood image
  image = read_texture("tex/maple.rgb", &width, &height, &components);
  glBindTexture(GL_TEXTURE_2D, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  free(image);
  
  // import maple wood highlight image
  image = read_texture("tex/mapleHighlight.rgb", &width, &height, &components);
  glBindTexture(GL_TEXTURE_2D, 2);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  free(image);
  
  // import maple wood fiber texture
  image = read_texture("tex/mapleFiber.rgb", &width, &height, &components);
  glBindTexture(GL_TEXTURE_2D, 3);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  free(image);
  
  // import maple wood image
  image = read_texture("tex/padauk.rgb", &width, &height, &components);
  glBindTexture(GL_TEXTURE_2D, 4);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  free(image);
  
  // import maple wood highlight image
  image = read_texture("tex/padaukHighlight.rgb", &width, &height, &components);
  glBindTexture(GL_TEXTURE_2D, 5);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  free(image);
  
  // import maple wood fiber texture
  image = read_texture("tex/padaukFiber.rgb", &width, &height, &components);
  glBindTexture(GL_TEXTURE_2D, 6);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  free(image);
  
  // create all the shaders
  createShaders();
}

// main running method
int main(int argc, char* argv[]){ 
  // set up GLUT
  glutInit(&argc, argv);
  
  // create the GLUT window with parameters
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_STENCIL | GLUT_DEPTH);
  glutInitWindowSize(1000, 600);
  glutInitWindowPosition(100,100);
  main_window = glutCreateWindow("Wood");
  init();
  //printf("came into main func");
  
  // set callbacks for different actions
  glutDisplayFunc(myGlutDisplay);
  GLUI_Master.set_glutReshapeFunc(myGlutReshape);
  GLUI_Master.set_glutIdleFunc(myGlutIdle);
  GLUI_Master.set_glutKeyboardFunc(myGlutKeyboard);
  GLUI_Master.set_glutMouseFunc(myGlutMouse);
  glutMotionFunc(myGlutMotion);
  
  // create the interface subwindow and add widgets
  glui = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_LEFT);
  
  // initialize live variables
  live_object_xz_trans[0] = 0;
  live_object_xz_trans[1] = 0;
  live_object_y_trans = 0;
  live_light_xz_trans[0] = 7;
  live_light_xz_trans[1] = 7;
  live_light_y_trans = 13;
  live_draw_floor = 1;
  live_draw_walls = 1;
  live_draw_object = 1;
  live_light_intensity = 1;
  
  // quit button
  glui->add_button("Quit", 0, (GLUI_Update_CB)exit);
  
  // empty space
  glui->add_statictext("");
  
  // the object rollout
  object_rollout = glui->add_rollout("Object");
  
  // object rotation and translation controls
  // we need an extra panel to keep things grouped properly
  GLUI_Panel *transform_panel = glui->add_panel_to_panel(object_rollout, "", GLUI_PANEL_NONE);
  object_rotation = glui->add_rotation_to_panel(transform_panel, "Rotation", live_object_rotation);
  object_rotation->reset();
  glui->add_column_to_panel(transform_panel, false);
  object_xz_trans = glui->add_translation_to_panel(transform_panel, "Translate XZ", GLUI_TRANSLATION_XY, live_object_xz_trans);
  glui->add_column_to_panel(transform_panel, false);
  object_y_trans =  glui->add_translation_to_panel(transform_panel, "Translate Y", GLUI_TRANSLATION_Y, &live_object_y_trans);
  object_xz_trans->scale_factor = 0.1f;
  object_y_trans->scale_factor = 0.1f;
  
  // empty space
  glui->add_statictext("");
  
  // lighting controls
  light_rollout = glui->add_rollout("Lighting Controls");
  GLUI_Panel *light_panel = glui->add_panel_to_panel(light_rollout, "", GLUI_PANEL_NONE);
  light_xz_trans = glui->add_translation_to_panel(light_panel, "Translate XZ", GLUI_TRANSLATION_XY, live_light_xz_trans);
  glui->add_column_to_panel(light_panel, false);
  light_y_trans =  glui->add_translation_to_panel(light_panel, "Translate Y", GLUI_TRANSLATION_Y, &live_light_y_trans);
  GLUI_Spinner *int_l = glui->add_spinner_to_panel(light_panel, "Intensity", GLUI_SPINNER_FLOAT, &live_light_intensity);
  int_l->set_float_limits(0.0, 1.0);
  
  glui->add_statictext("");
  
  // our checkbox options for deciding what to draw
  glui->add_checkbox("Draw Floor", &live_draw_floor);
  glui->add_checkbox("Draw Walls", &live_draw_walls);
  glui->add_checkbox("Draw Object", &live_draw_object);
  
  // empty space
  glui->set_main_gfx_window(main_window);
  
  // initialize the camera
  eye[0] = -3;
  eye[1] = 13;
  eye[2] = 7;
  lookat[0] = 0;
  lookat[1] = 0;
  lookat[2] = 0;
  
  // activate GL modes
  glEnable(GL_DEPTH_TEST);
  
  // give control over to GLUT for continuous drawing
  glutMainLoop(); 
  return 0;
}
