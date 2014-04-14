#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>

#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <GL/glut.h>
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glut32.lib")
#elif __APPLE__
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif

#include "FrameXform.h"
#include "WaveFrontOBJ.h"
#ifndef M_PI

#define M_PI 3.1415926535897932384626433832795
#endif

// 'cameras' stores infomation of 5 cameras.
double cameras[5][9] = 
{
	{28,18,28, 0,2,0, 0,1,0},   
	{28,18,-28, 0,2,0, 0,1,0}, 
	{-28,18,28, 0,2,0, 0,1,0}, 
	{-12,12,0, 0,2,0, 0,1,0},  
	{0,100,0,  0,0,0, 1,0,0}
};
int cameraCount = sizeof( cameras ) / sizeof( cameras[0] );

int cameraIndex, camID;
vector<FrameXform> wld2cam, cam2wld; 
WaveFrontOBJ* cam;

// Variables for 'cow' object.
FrameXform cow2wld;
WaveFrontOBJ* cow;
int cowID;

// Variables for 'beethovan' object.
FrameXform bet2wld;
WaveFrontOBJ* bet;
int betID;


unsigned floorTexID;
int frame = 0;
int width, height;
int selectMode, oldX, oldY;

// Variables for rotation
bool isRotation = false;
double rotateX = 0;
double rotateY = 0;
double rotateZ = 0;
double spin = 0;

// Variables for translation
enum AxisTranslation
{
	kNone /* 이동이 되지 않는 상태*/,
	kXAxis /* X축으로 이동을 하는 상태*/,
	kYAxis /* Y축으로 이동을 하는 상태*/,
	kZAxis /* Z축으로 이동ㅇ르 하는 상태*/
};

AxisTranslation axis = kNone;
double deltaX = 0;
double deltaY = 0;
double deltaZ = 0;

// Variables for view, model changing
enum TransformationSpace
{
	kModel,
	kView
};
TransformationSpace space = kModel;

void drawFrame(float len);

//------------------------------------------------------------------------------
void munge( int x, double& r, double& g, double& b)
{
	r = (x & 255)/double(255);
	g = ((x >> 8) & 255)/double(255);
	b = ((x >> 16) & 255)/double(255);
}

//------------------------------------------------------------------------------
int unmunge( double r, double g, double b)
{
	return (int(r) + (int(g) << 8) + (int(b) << 16));
}

//------------------------------------------------------------------------------
void setCamera()
{
	int i;
	if (frame == 0)
	{
		// intialize camera model.
		cam = new WaveFrontOBJ("camera.obj");	// Read information of camera from camera.obj.
		camID = glGenLists(1);					// Create display list of the camera.
		glNewList(camID, GL_COMPILE);			// Begin compiling the display list using camID.
		cam->Draw();							// Draw the camera. you can do this job again through camID..
		glEndList();							// Terminate compiling the display list.

		// initialize camera frame transforms.
		for (i=0; i < cameraCount; i++ )
		{
			double* c = cameras[i];											// 'c' points the coordinate of i-th camera.
			wld2cam.push_back(FrameXform());								// Insert {0} matrix to wld2cam vector.
			glPushMatrix();													// Push the current matrix of GL into stack.
			glLoadIdentity();												// Set the GL matrix Identity matrix.
			gluLookAt(c[0],c[1],c[2], c[3],c[4],c[5], c[6],c[7],c[8]);		// Setting the coordinate of camera.
			glGetDoublev( GL_MODELVIEW_MATRIX, wld2cam[i].matrix() );		// Read the world-to-camera matrix computed by gluLookAt.
			glPopMatrix();													// Transfer the matrix that was pushed the stack to GL.
			cam2wld.push_back(wld2cam[i].inverse());						// Get the camera-to-world matrix.
		}
		cameraIndex = 0;
	}

	// set viewing transformation.
	glLoadMatrixd(wld2cam[cameraIndex].matrix());

	// draw other cameras.
	for (i=0; i < (int)wld2cam.size(); i++ )
	{
		if (i != cameraIndex)
		{
			glPushMatrix();												// Push the current matrix on GL to stack. The matrix is wld2cam[cameraIndex].matrix().
			glMultMatrixd(cam2wld[i].matrix());							// Multiply the matrix to draw i-th camera.
			if (selectMode == 0)										// selectMode == 1 means backbuffer mode.
			{
				drawFrame(5);											// Draw x, y, and z axis.
				float frontColor[] = {0.2, 0.2, 0.2, 1.0};
				glEnable(GL_LIGHTING);									
				glMaterialfv(GL_FRONT, GL_AMBIENT, frontColor);			// Set ambient property frontColor.
				glMaterialfv(GL_FRONT, GL_DIFFUSE, frontColor);			// Set diffuse property frontColor.
			}
			else
			{
				double r,g,b;				
				glDisable(GL_LIGHTING);									// Disable lighting in backbuffer mode.
				munge(i+1, r,g,b);										// Match the corresponding (i+1)th color to r, g, b. You can change the color of camera on backbuffer.
				glColor3f(r, g, b);										// Set r, g, b the color of camera.
			}
			glScaled(0.5,0.5,0.5);										// Reduce camera size by 1/2.
			glTranslated(1.1,1.1,0.0);									// Translate it (1.1, 1.1, 0.0).
			glCallList(camID);											// Re-draw using display list from camID. 
			glPopMatrix();												// Call the matrix on stack. wld2cam[cameraIndex].matrix() in here.
		}
	}
}

// Idle rotation Animation Function
void rotationDisplay()
{
	spin = (spin + 1.0);
	if (spin > 360)
		spin = spin - 360.0;

	glutPostRedisplay();
}

void drawAxisOfRotation(float length)
{
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
	glColor3d(1,1,1);
	glVertex3d(length*rotateX,length*rotateY,length*rotateZ);			
	glVertex3d(-length*rotateX,-length*rotateY,-length*rotateZ);			
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);
}


/*********************************************************************************
* Draw x, y, z axis of current frame on screen.
* x, y, and z are corresponded Red, Green, and Blue, resp.
**********************************************************************************/
void drawFrame(float len)
{
	glDisable(GL_LIGHTING);		// Lighting is not needed for drawing axis.
	glBegin(GL_LINES);			// Start drawing lines.
	glColor3d(1,0,0);			// color of x-axis is red.
	glVertex3d(0,0,0);			
	glVertex3d(len,0,0);		// Draw line(x-axis) from (0,0,0) to (len, 0, 0). 
	glColor3d(0,1,0);			// color of y-axis is green.
	glVertex3d(0,0,0);			
	glVertex3d(0,len,0);		// Draw line(y-axis) from (0,0,0) to (0, len, 0).
	glColor3d(0,0,1);			// color of z-axis is  blue.
	glVertex3d(0,0,0);
	glVertex3d(0,0,len);		// Draw line(z-axis) from (0,0,0) - (0, 0, len).
	glEnd();					// End drawing lines.
}

/*********************************************************************************
* Draw 'cow' object.
**********************************************************************************/
void drawCow()
{  
	if (frame == 0)
	{
		// Initialization part.

		// Read information from cow.obj.
		cow = new WaveFrontOBJ( "cow.obj" );

		// Make display list. After this, you can draw cow using 'cowID'.
		cowID = glGenLists(1);				// Create display lists
		glNewList(cowID, GL_COMPILE);		// Begin compiling the display list using cowID
		cow->Draw();						// Draw the cow on display list.
		glEndList();						// Terminate compiling the display list. Now, you can draw cow using 'cowID'.
		glPushMatrix();						// Push the current matrix of GL into stack.
		glLoadIdentity();					// Set the GL matrix Identity matrix.
		glTranslated(0,-cow->bbmin.y,-8);	// Set the location of cow.
		glRotated(-90, 0, 1, 0);			// Set the direction of cow. These information are stored in the matrix of GL.
		glGetDoublev(GL_MODELVIEW_MATRIX, cow2wld.matrix());	// Read the modelview matrix about location and direction set above, and store it in cow2wld matrix.
		glPopMatrix();						// Pop the matrix on stack to GL.
	}

	glPushMatrix();		// Push the current matrix of GL into stack. This is because the matrix of GL will be change while drawing cow.

	glMultMatrixd(wld2cam[cameraIndex].matrix());
	glTranslated(deltaX, deltaY, deltaZ); // To move the cow model.
	glMultMatrixd(cam2wld[cameraIndex].matrix());
	// The information about location of cow to be drawn is stored in cow2wld matrix.
	// (Project2 hint) If you change the value of the cow2wld matrix or the current matrix, cow would rotate or move.
	glMultMatrixd(cow2wld.matrix());


	if (selectMode == 0)									// selectMode == 1 means backbuffer mode.
	{
		drawFrame(5);										// Draw x, y, and z axis.
		float frontColor[] = {0.8, 0.2, 0.9, 1.0};
		glEnable(GL_LIGHTING);
		glMaterialfv(GL_FRONT, GL_AMBIENT, frontColor);		// Set ambient property frontColor.
		glMaterialfv(GL_FRONT, GL_DIFFUSE, frontColor);		// Set diffuse property frontColor.
	}
	else
	{
		double r,g,b;  
		glDisable(GL_LIGHTING);								// Disable lighting in backbuffer mode.
		munge(32, r,g,b );									// Match the corresponding constant color to r, g, b. You can change the color of camera on backbuffer
		glColor3d(r, g, b);									
	}

	glCallList(cowID);		// Draw cow. 
	glPopMatrix();			// Pop the matrix in stack to GL. Change it the matrix before drawing cow.
}

/*********************************************************************************
* Draw 'beethovan' object.
**********************************************************************************/
void drawBet()
{  
	if (frame == 0)
	{
		// Initialization part.

		// Read information from beethovan.obj.
		bet = new WaveFrontOBJ( "beethovan.obj" );

		// Make display list. After this, you can draw beethovan using 'betID'.
		betID = glGenLists(1);
		glNewList(betID, GL_COMPILE);
		bet->Draw();
		glEndList();
		glPushMatrix();
		glLoadIdentity();
		glTranslated(0,-bet->bbmin.y,8);
		glRotated(180, 0, 1, 0);
		glGetDoublev(GL_MODELVIEW_MATRIX, bet2wld.matrix());
		glPopMatrix();
	}

	glPushMatrix();
	glMultMatrixd(bet2wld.matrix());
	if (selectMode == 0)
	{
		drawFrame(8);
		float frontColor[] = {0.8, 0.3, 0.1, 1.0};
		glEnable(GL_LIGHTING);
		glMaterialfv(GL_FRONT, GL_AMBIENT, frontColor);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, frontColor);
	}
	else
	{
		double r,g,b;  
		glDisable(GL_LIGHTING);
		munge(33, r,g,b );
		glColor3d(r, g, b);    
	}
	glCallList(betID);
	glPopMatrix();
}


/*********************************************************************************
* Draw floor on 3D plane.
**********************************************************************************/
void drawFloor()
{  
	if (frame == 0)
	{
		// Initialization part.
		// After making checker-patterned texture, use this repetitively.

		// Insert color into checker[] according to checker pattern.
		const int size = 8;
		unsigned char checker[size*size*3];
		for( int i=0; i < size*size; i++ )
		{
			if (((i/size) ^ i) & 1)
			{
				checker[3*i+0] = 200;
				checker[3*i+1] = 32;
				checker[3*i+2] = 32;
			}
			else
			{
				checker[3*i+0] = 200;
				checker[3*i+1] = 200;
				checker[3*i+2] = 32;
			}
		}

		// Make texture which is accessible through floorTexID. 
		glGenTextures( 1, &floorTexID );				
		glBindTexture(GL_TEXTURE_2D, floorTexID);		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, checker);
	}

	glDisable(GL_LIGHTING);

	// Set background color.
	if (selectMode == 0)
		glColor3d(0.35, .2, 0.1);
	else
	{
		// In backbuffer mode.
		double r,g,b;
		munge(34, r,g,b);
		glColor3d(r, g, b);
	}

	// Draw background rectangle. 
	glBegin(GL_POLYGON);
	glVertex3f( 2000,-0.2, 2000);
	glVertex3f( 2000,-0.2,-2000);
	glVertex3f(-2000,-0.2,-2000);
	glVertex3f(-2000,-0.2, 2000);
	glEnd();

	
	// Set color of the floor.
	if (selectMode == 0)
	{
		// Assign checker-patterned texture.
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorTexID );
	}
	else
	{
		// Assign color on backbuffer mode.
		double r,g,b;
		munge(35, r,g,b);
		glColor3d(r, g, b);
	}

	// Draw the floor. Match the texture's coordinates and the floor's coordinates resp. 
	glBegin(GL_POLYGON);
	glTexCoord2d(0,0);
	glVertex3d(-12,-0.1,-12);		// Texture's (0,0) is bound to (-12,-0.1,-12).
	glTexCoord2d(1,0);
	glVertex3d( 12,-0.1,-12);		// Texture's (1,0) is bound to (12,-0.1,-12).
	glTexCoord2d(1,1);
	glVertex3d( 12,-0.1, 12);		// Texture's (1,1) is bound to (12,-0.1,12).
	glTexCoord2d(0,1);
	glVertex3d(-12,-0.1, 12);		// Texture's (0,1) is bound to (-12,-0.1,12).
	glEnd();

	if (selectMode == 0)
	{
		glDisable(GL_TEXTURE_2D);	
		drawFrame(5);				// Draw x, y, and z axis.
	}
}


/*********************************************************************************
* Call this part whenever display events are needed. 
* Display events are called in case of re-rendering by OS. ex) screen movement, screen maximization, etc.
* Or, user can occur the events by using glutPostRedisplay() function directly.
* this part is called in main() function by registering on glutDisplayFunc(display).
**********************************************************************************/
void display()
{
	// selectMode == 1 means backbuffer mode.
	if (selectMode == 0)
		glClearColor(0, 0.6, 0.8, 1);								// Clear color setting
	else
		glClearColor(0, 0, 0, 1);									// When the backbuffer mode, clear color is set to black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear the screen
	setCamera();													// Locate the camera's position, and draw all of them.

	drawFloor();													// Draw floor.
	drawCow();														// Draw cow.
	//drawBet();


	glFlush();

	// If it is not backbuffer mode, swap the screen. In backbuffer mode, this is not necessary because it is not presented on screen.
	if (selectMode == 0)
		glutSwapBuffers();
	frame += 1;					
}


/*********************************************************************************
* Call this part whenever size of the window is changed. 
* This part is called in main() function by registering on glutReshapeFunc(reshape).
**********************************************************************************/
void reshape( int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);            // Select The Projection Matrix
	glLoadIdentity();                       // Reset The Projection Matrix
	// Define perspective projection frustum
	double aspect = width/double(height);
	gluPerspective(45, aspect, 1, 1024);
	glMatrixMode(GL_MODELVIEW);             // Select The Modelview Matrix
	glLoadIdentity();                       // Reset The Projection Matrix
}

//------------------------------------------------------------------------------
void initialize()
{
	// Set up OpenGL state
	glShadeModel(GL_SMOOTH);         // Set Smooth Shading
	glEnable(GL_DEPTH_TEST);         // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);          // The Type Of Depth Test To Do
	// Use perspective correct interpolation if available
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	// Initialize the matrix stacks
	reshape(width, height);
	// Define lighting for the scene
	float lightDirection[]   = {1.0, 1.0, 1.0, 0};
	float ambientIntensity[] = {0.1, 0.1, 0.1, 1.0};
	float lightIntensity[]   = {0.9, 0.9, 0.9, 1.0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
	glLightfv(GL_LIGHT0, GL_POSITION, lightDirection);
	glEnable(GL_LIGHT0);
}

/*********************************************************************************
* Call this part whenever mouse button is clicked.
* This part is called in main() function by registering on glutMouseFunc(onMouseButton).
**********************************************************************************/
void onMouseButton(int button, int state, int x, int y)
{
	y = height - y - 1;
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			printf( "Left mouse click at (%d, %d)\n", x, y );

			// (Project 4) After drawing object on backbuffer, you can recognize which object is selected by reading pixel of (x, y).
			// Change the value of selectMode to 1, then draw the object on backbuffer when display() function is called. 
			selectMode = 1;
			display();
			glReadBuffer(GL_BACK);
			unsigned char pixel[3];
			glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
			printf( "pixel = %d\n", unmunge(pixel[0],pixel[1],pixel[2]));
			selectMode = 0;

			// (Project 4) TODO : Perform the proper task about selected object.
			// hint : you can recognize which object is selected by pixel value.

			// Save current clicked location of mouse here, and then use this on onMouseDrag function. 
			oldX = x;
			oldY = y;
		}
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		printf( "Right mouse click at (%d, %d)\n",x,y );
	}
	glutPostRedisplay();
}



/*********************************************************************************
* Call this part whenever user drags mouse. 
* Input parameters x, y are coordinate of mouse on dragging. 
* Value of global variables oldX, oldY is stored on onMouseButton, 
* Then, those are used to verify value of x - oldX,  y - oldY to know its movement.
**********************************************************************************/
void onMouseDrag(int x, int y)
{
	y = height - y - 1;

	switch(axis)
	{
		case kXAxis:
		case kYAxis:
		{
			deltaX = (x - oldX) / 10.0;
			deltaY = (y - oldY) / 10.0;
			break;
		}
		case kZAxis:
		{
			deltaZ = (x - oldX) / 10.0;
			break;
		}
		default:
		{
			printf("Axis is not selected");	
			break;
		}
	}

	// (Project 2,3,4) TODO : Implement here to perform properly when drag the mouse on each cases resp.

	glutPostRedisplay();
}

/*********************************************************************************
* Call this part whenever user types keyboard. 
* This part is called in main() function by registering on glutKeyboardFunc(onKeyPress).
**********************************************************************************/
void onKeyPress( unsigned char key, int x, int y)
{
	axis = kNone;

	// If 'c' or space bar are pressed, alter the camera.
	// If a number is pressed, alter the camera corresponding the number.
	if ((key == ' ') || (key == 'c'))
	{    
		printf( "Toggle camera %d\n", cameraIndex );
		cameraIndex += 1;
	}      
	else if ((key >= '0') && (key <= '9'))
	{
		cameraIndex = key - '0';
	}
	// If 'r' or 'R' are pressed, toggle isRotation variable.
	// If isRotation is true, it makes randomly axis of rotation.
	else if ((key == 'r') || (key == 'R'))
	{
		isRotation = !isRotation;

		if (isRotation)
		{
			srand(time(NULL));
  
  		rotateX = (rand() % 1000) / 1000.0;
  		rotateY = (rand() % 1000) / 1000.0;
  		rotateZ = (rand() % 1000) / 1000.0;
  		
  		printf( "double value %f %f %f\n", rotateX, rotateY, rotateZ );
  		
			glutIdleFunc(rotationDisplay);
		}
		else
		{
			glutIdleFunc(NULL);
			return;
		}
	}
	else if ((key == 'm') || (key == 'M'))
	{
		space = kModel;
	}
	else if ((key == 'v') || (key == 'V'))
	{
		space = kView;
	}
	// If 'x' or 'y' or 'z' are pressed, it changes axis of translation.
	// If 'x' is pressed, it makes x axis become axis of translation
	else if ((key == 'x') || (key == 'X'))
	{
		axis = kXAxis;
		return;
	}
	// If 'y' is pressed, it makes y axis become axis of translation
	else if ((key == 'y') || (key == 'Y'))
	{
		axis = kYAxis;
		return;
	}
	// If 'z' is pressed, it makes z axis become axis of translation
	else if ((key == 'z') || (key == 'Z'))
	{
		axis = kZAxis;
		return;
	}


	if (cameraIndex >= (int)wld2cam.size() )
		cameraIndex = 0;
	
	glutPostRedisplay();
}

//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	width = 800;
	height = 600;
	frame = 0;
	glutInit( &argc, argv );						// Initialize openGL.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);	// Initialize display mode. This project will use double buffer and RGB color.
	glutInitWindowSize(width, height);				// Initialize window size.
	glutInitWindowPosition(100, 100);				// Initialize window coordinate.
	glutCreateWindow("Simple Scene");				// Make window whose name is "Simple Scene".
	glutDisplayFunc(display);						// Register display function to call that when drawing screen event is needed.
	glutReshapeFunc(reshape);						// Register reshape function to call that when size of the window is changed.
	glutKeyboardFunc(onKeyPress);					// Register onKeyPress function to call that when user presses the keyboard.
	glutMouseFunc(onMouseButton);					// Register onMouseButton function to call that when user moves mouse.
	glutMotionFunc(onMouseDrag);					// Register onMouseDrag function to call that when user drags mouse.
	int rv,gv,bv;
	glGetIntegerv(GL_RED_BITS,&rv);					// Get the depth of red bits from GL.
	glGetIntegerv(GL_GREEN_BITS,&gv);				// Get the depth of green bits from GL.
	glGetIntegerv(GL_BLUE_BITS,&bv);				// Get the depth of blue bits from GL.
	printf( "Pixel depth = %d : %d : %d\n", rv, gv, bv );
	initialize();									// Initialize the other thing.
	glutMainLoop();									// Execute the loop which handles events.

	return 0;
}
