 
#include "GL/glew.h"
#include <stdio.h>
#include <iostream>
#include "GL/glut.h" 
#include "GL/glui.h"
 
#include <math.h> 
 
using namespace std;

int	main_window; 
 
// the camera info   
float eye[3]; 
float lookat[3]; 
 
 
// pointers for all of the glui controls 
GLUI *glui; 
GLUI_Rollout		*object_rollout; 
GLUI_RadioGroup		*object_type_radio; 
GLUI_Rotation		*object_rotation; 
GLUI_Translation	*object_xz_trans; 
GLUI_Translation	*object_y_trans; 
 
GLUI_Rollout		*anim_rollout; 
GLUI_Button			*action_button; 
 
GLUI_Checkbox *draw_floor_check; 
GLUI_Checkbox *draw_object_check; 
 
// This  checkbox utilizes the callback 
GLUI_Checkbox *use_depth_buffer; 
 
 
// the user id's that we can use to identify which control 
// caused the callback to be called 
#define CB_DEPTH_BUFFER 0 
#define CB_ACTION_BUTTON 1 
#define CB_RESET 2 
 
// walking action variables 
// 
GLfloat step = 0; 
GLfloat live_anim_speed = 3; 
 
// live variables 
// each of these are associated with a control in the interface. 
// when the control is modified, these variables are automatically updated 
int live_object_type;	// 0=cube, 1=sphere, 2=torus 
float live_object_rotation[16]; 
float live_object_xz_trans[2]; 
float live_object_y_trans; 
int live_draw_floor; 
int live_draw_object; 
int live_use_depth_buffer; 
 
 
 
void normalize(float v[3]) 
{ 
	float l = v[0]*v[0] + v[1]*v[1] + v[2]*v[2]; 
	l = 1 / (float)sqrt(l); 
 
	v[0] *= l; 
	v[1] *= l; 
	v[2] *= l; 
} 
 
void crossproduct(float a[3], float b[3], float res[3]) 
{ 
	res[0] = (a[1] * b[2] - a[2] * b[1]); 
	res[1] = (a[2] * b[0] - a[0] * b[2]); 
	res[2] = (a[0] * b[1] - a[1] * b[0]); 
} 
 
float length(float v[3]) 
{ 
	return (float)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]); 
} 
 
 
void myGlutIdle(void) 
{ 
	// make sure the main window is active 
	if (glutGetWindow() != main_window) 
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
 
// catch mouse up/down events 
void myGlutMouse(int button, int state, int x, int y) 
{ 
	if (state == GLUT_DOWN) 
		cur_button = button; 
	else 
	{ 
		if (button == cur_button) 
			cur_button = -1; 
	} 
 
	last_x = x; 
	last_y = y; 
} 
 
// catch mouse move events 
void myGlutMotion(int x, int y) 
{ 
	// the change in mouse position 
	int dx = x-last_x; 
	int dy = y-last_y; 
 
	float scale, len, theta; 
	float neye[3], neye2[3]; 
	float f[3], r[3], u[3]; 
 
	switch(cur_button) 
	{ 
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
		if (len < 1) 
		{ 
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
 
// you can put keyboard shortcuts in here 
void myGlutKeyboard(unsigned char key, int x, int y) 
{ 
	switch(key) 
	{ 
	// quit 
	case 27:  
	case 'q': 
	case 'Q': 
		exit(0); 
		break; 
	} 
 
	glutPostRedisplay(); 
} 
 
// the window has changed shapes, fix ourselves up 
void myGlutReshape(int	x, int y) 
{ 
	int tx, ty, tw, th; 
	GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th); 
	glViewport(tx, ty, tw, th); 
 
	glutPostRedisplay(); 
} 
 
 
// draw the scene 
void myGlutDisplay(	void ) 
{ 
	glClearColor(0, 0, 0, 0); 
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); 
 
	// projection transform 
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity(); 
	glFrustum(-1, 1, -1, 1, 1, 1000); 
 
	// camera transform 
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity(); 
	gluLookAt(eye[0], eye[1], eye[2], lookat[0], lookat[1], lookat[2], 0, 1, 0); 
 
	 
	// 
	// draw some stuff 
	// 
 
	if (live_draw_floor) 
	{ 
		glBegin(GL_TRIANGLE_FAN); 
		glColor3f(0.4f, 0.4f, 0.4f);    // note: color is state and only needs to be set once 
		glVertex3f(-10, 0, -10); 
		glVertex3f( 10, 0, -10); 
		glVertex3f( 10, 0,  10); 
		glVertex3f(-10, 0,  10); 
		glEnd(); 
	} 
 
	if (live_draw_object) 
	{ 
		glColor3f(0, 1, 1); 
		 
		glPushMatrix(); 
		glTranslatef(live_object_xz_trans[0], live_object_y_trans, -live_object_xz_trans[1]); 
		glMultMatrixf(live_object_rotation); 
 
		switch(live_object_type) 
		{ 
		case 0: 
			glutSolidCube(2); 
			break; 
		case 1: 
			glutSolidSphere(2, 30, 30); 
			break; 
		case 2: 
			glutSolidTorus(0.5, 2, 30, 30); 
			break; 
		} 
 
		glPopMatrix(); 
	} 
 
	glutSwapBuffers();  
} 
 
// some controls generate a callback when they are changed 
void glui_cb(int control) 
{ 
 
	switch(control) 
	{ 
	case CB_DEPTH_BUFFER: 
		if (live_use_depth_buffer) 
			glEnable(GL_DEPTH_TEST); 
		else 
			glDisable(GL_DEPTH_TEST); 
		break; 
	case CB_ACTION_BUTTON: 
		if (step < 0) 
		{ 
			step = 0; 
			action_button->set_name("Stop"); 
		} 
		else 
		{ 
			step = -1; 
			action_button->set_name("Start"); 
		} 
		break; 
	case CB_RESET: 
		// 
		// put your reset callback here 
		// 
		break; 
	} 
 
	glutPostRedisplay(); 
} 
void init( void)
{
	GLenum err = glewInit();
	if(GLEW_OK != err)
	{
		//failed to initialize GLEW!
		printf("failed !!!!!!!!!!!!");
	}
	printf("%s",glewGetString(GLEW_VERSION));
} 
// entry point 
int main(int argc,	char* argv[]) 
{ 
 
        glutInit(&argc, argv); 
	// 
	// create the glut window 
	// 
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH); 
	glutInitWindowSize(1000, 600); 
	glutInitWindowPosition(100,100); 
	main_window = glutCreateWindow("Sample Interface"); 
	init();
	printf("came into main func"); 
	// 
	// set callbacks 
	// 
	glutDisplayFunc(myGlutDisplay); 
	GLUI_Master.set_glutReshapeFunc(myGlutReshape); 
	GLUI_Master.set_glutIdleFunc(myGlutIdle); 
	GLUI_Master.set_glutKeyboardFunc(myGlutKeyboard); 
	GLUI_Master.set_glutMouseFunc(myGlutMouse); 
	glutMotionFunc(myGlutMotion); 
 
 
	// 
	// create the interface subwindow and add widgets 
	// 
	glui = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_LEFT); 
 
 
	// initialize live variables 
	live_object_type = 0; 
	live_object_xz_trans[0] = 0; 
	live_object_xz_trans[1] = 0; 
	live_object_y_trans = 0; 
	live_draw_floor = 1; 
	live_draw_object = 1; 
	live_use_depth_buffer = 1; 
 
	// quit button 
	glui->add_button("Quit", 0, (GLUI_Update_CB)exit); 
 
	// empty space 
	glui->add_statictext(""); 
 
	// the object rollout 
	object_rollout = glui->add_rollout("Object"); 
 
	// the radio buttons 
	object_type_radio = glui->add_radiogroup_to_panel(object_rollout, &live_object_type); 
	glui->add_radiobutton_to_group(object_type_radio, "cube"); 
	glui->add_radiobutton_to_group(object_type_radio, "sphere"); 
	glui->add_radiobutton_to_group(object_type_radio, "torus"); 
 
	// rotation and translation controls 
	// we need an extra panel to keep things grouped properly 
	GLUI_Panel *transform_panel = glui->add_panel_to_panel(object_rollout, "", GLUI_PANEL_NONE); 
	object_rotation = glui->add_rotation_to_panel(transform_panel, "Rotation", live_object_rotation); 
	object_rotation->reset(); 
 
	glui->add_column_to_panel(transform_panel, false); 
	object_xz_trans = glui->add_translation_to_panel(transform_panel, "Translate XZ",  
													 GLUI_TRANSLATION_XY, live_object_xz_trans); 
 
	glui->add_column_to_panel(transform_panel, false); 
	object_y_trans =  glui->add_translation_to_panel(transform_panel, "Translate Y",  
													 GLUI_TRANSLATION_Y, &live_object_y_trans); 
 
	object_xz_trans->scale_factor = 0.1f; 
	object_y_trans->scale_factor = 0.1f; 
 
	glui->add_button_to_panel(object_rollout, "Reset Object Transform", CB_RESET, glui_cb); 
 
	// empty space 
	glui->add_statictext(""); 
 
	// the walk control 
	anim_rollout = glui->add_rollout("Animation"); 
 
	action_button = glui->add_button_to_panel(anim_rollout, "Stop", CB_ACTION_BUTTON, glui_cb); 
 
	GLUI_Spinner *spin_s = 
 
		glui->add_spinner_to_panel(anim_rollout, "Speed", GLUI_SPINNER_FLOAT, &live_anim_speed); 
 
	spin_s->set_float_limits(0.1, 10.0); 
 
	// our checkbox options for deciding what to draw 
	glui->add_checkbox("Draw Floor", &live_draw_floor); 
	glui->add_checkbox("Draw Object", &live_draw_object); 
 
	// empty space 
	glui->add_statictext(""); 
 
	glui->add_checkbox("Use Depth Buffer", &live_use_depth_buffer, CB_DEPTH_BUFFER, glui_cb); 
 
	glui->set_main_gfx_window(main_window); 
 
	// initialize the camera 
	eye[0] = 0; 
	eye[1] = 4; 
	eye[2] = 10; 
	lookat[0] = 0; 
	lookat[1] = 0; 
	lookat[2] = 0; 
 
	// initialize gl 
	glEnable(GL_DEPTH_TEST); 
	glEnable(GL_COLOR_MATERIAL); 
 
	// give control over to glut 
	glutMainLoop(); 
 
	return 0; 
} 
 
 
 
 
 
 
 
 
 
 
 
