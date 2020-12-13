#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.

// title of these windows:

const char *WINDOWTITLE = { "OpenGL / GLUT Sample -- Jianlong Graphics" };
const char *GLUITITLE   = { "User Interface Window" };

// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };

// the escape key:

#define ESCAPE		0x1b

// initial window size:

const int INIT_WINDOW_SIZE = { 600 };

// size of the 3d box:

const float BOXSIZE = { 2.f };

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

// minimum allowable scale factor:

const float MINSCALE = { 0.05f };

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = { 3 };
const int SCROLL_WHEEL_DOWN = { 4 };

// equivalent mouse movement when we click a the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = { 5. };

// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };

// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta",
	(char*)"White",
	(char*)"Black"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong

//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

// should we turn the shadows on?

//#define ENABLE_SHADOWS



// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
GLuint	BoxList;				// object display list
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoShadowMenu();
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );
void	Passive_motion(int, int);
void	Camera();
void	FP_rotate();

void			Axes( float );
unsigned char *	BmpToTexture( char *, int *, int * );
void			HsvRgb( float[3], float [3] );
int				ReadInt( FILE * );
short			ReadShort( FILE * );

void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);

// main program:

//load obj with mtl
#include "loadobj_mtl.h"

//texture
unsigned char* Texture, *Texture1, *Texture2;
int width, height, width1, height1, width2, height2;
GLuint Tex0, Tex1, Tex2;

//lighting 
float*
Array3(float a, float b, float c)
{
	static float array[4];
	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}
// utility to create an array from a multiplier and an array:
float*
MulArray3(float factor, float array0[3])
{
	static float array[4];
	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}

void
SetPointLight(int ilight, float x, float y, float z, float r, float g, float b)
{
	glLightfv(ilight, GL_POSITION, Array3(x, y, z));
	glLightfv(ilight, GL_AMBIENT, Array3(0., 0., 0.));
	glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
	glLightfv(ilight, GL_SPECULAR, Array3(r, g, b));
	glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
	glLightf(ilight, GL_LINEAR_ATTENUATION, 0.);
	glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
	glEnable(ilight);
}

void
SetMaterial(float r, float g, float b, float shininess)
{
	glMaterialfv(GL_BACK, GL_EMISSION, Array3(0., 0., 0.));
	glMaterialfv(GL_BACK, GL_AMBIENT, MulArray3(.4f, White));
	glMaterialfv(GL_BACK, GL_DIFFUSE, MulArray3(1., White));
	glMaterialfv(GL_BACK, GL_SPECULAR, Array3(0., 0., 0.));
	glMaterialf(GL_BACK, GL_SHININESS, 2.f);
	glMaterialfv(GL_FRONT, GL_EMISSION, Array3(0., 0., 0.));
	glMaterialfv(GL_FRONT, GL_AMBIENT, Array3(r, g, b));
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Array3(r, g, b));
	glMaterialfv(GL_FRONT, GL_SPECULAR, MulArray3(.8f, White));
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

//animation
int tt = 0;
int il = 300;
bool inAnimation = FALSE;
int elp_time;

//sound
#include <mmsystem.h>

//First person
float pitch = 0.0, yaw = 0.0;
bool FirstPerson = FALSE;
#include "sphere.h"

//Particle
#include <ctime>
#include <glm.hpp>

int last_elp = 0;
int elp = 0;
bool showFire = FALSE;

int ran_int(int HI, int LO)
{
	return LO + rand() % (HI - LO);
}

float ran_float(float HI, float LO)
{
	return LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
}

struct Particle {
	glm::vec3 Position, Velocity;
	glm::vec4 Color;
	int     Life;
};

std::vector<struct Particle*> particles;
std::vector<struct Particle *> particles_ex;
int num_particles = 1000;

void update_particles()
{
	glm::vec3 p_center = { 0., 0., 0. };
	float radius_1 = 0.07;
	float radius_2 = 0.05;
	int idx = 0;
	while (1)
	{
		if (idx >= particles.size())
			break;
		particles[idx]->Life -= elp;
		if (particles[idx]->Life <= 0)
		{
			free(particles[idx]);
			particles.erase(particles.begin() + idx);
		}
		else
		{
			particles[idx]->Position = particles[idx]->Position + (float)elp * particles[idx]->Velocity;
			idx++;
		}
	}

	for (int i = 0; i < num_particles / 2; i++)
	{
		Particle *p = new Particle();

		p->Color[0] = 1.;
		p->Color[1] = 0.;
		p->Color[2] = 0.;
		p->Color[3] = 0.2;

		p->Position[0] = ran_float(radius_1, -radius_1);
		p->Position[1] = 0.;
		p->Position[2] = ran_float(radius_1, -radius_1);

		glm::vec3 dis = p->Position - p_center;
		float dis_c = dot(dis, dis);

		p->Velocity[0] = 0.;
		p->Velocity[1] = ran_float(radius_1 * radius_1 - dis_c, 0.);
		p->Velocity[2] = 0.;

		p->Life = ran_int(il / 5, 0);
		
		if (dot(dis, dis) <= (radius_1 * radius_1))
			particles.push_back(p);
		else
			free(p);
	}

	for (int i = 0; i < num_particles / 2; i++)
	{
		Particle* p = new Particle();

		p->Color[0] = 1.;
		p->Color[1] = 0.67;
		p->Color[2] = 0.;
		p->Color[3] = 0.4;

		p->Position[0] = ran_float(radius_2, -radius_2);
		p->Position[1] = 0.;
		p->Position[2] = ran_float(radius_2, -radius_2);

		glm::vec3 dis = p->Position - p_center;
		float dis_c = dot(dis, dis);

		p->Velocity[0] = 0.;
		p->Velocity[1] = ran_float(radius_2 * radius_2 - dis_c, 0);
		p->Velocity[2] = 0.;

		p->Life = ran_int(il / 3, 0);

		if (dot(dis, dis) <= (radius_2 * radius_2))
			particles.push_back(p);
		else 
			free(p);

	}
}

void update_particles_ex()
{
	glm::vec3 p_center = { 0., 0., 0. };
	float radius_1 = 0.07;
	float radius_2 = 0.05;
	int idx = 0;
	while (1)
	{
		if (idx >= particles_ex.size())
			break;
		particles_ex[idx]->Life -= elp;
		if (particles_ex[idx]->Life <= 0)
		{
			free(particles_ex[idx]);
			particles_ex.erase(particles_ex.begin() + idx);
		}
		else
		{
			particles_ex[idx]->Position = particles_ex[idx]->Position + (float)elp * particles_ex[idx]->Velocity;
			idx++;
		}
	}

	for (int i = 0; i < num_particles / 2; i++)
	{
		Particle* p = new Particle();

		p->Color[0] = 1.;
		p->Color[1] = 0.;
		p->Color[2] = 0.;
		p->Color[3] = 0.5;

		p->Position[0] = ran_float(radius_1, -radius_1);
		p->Position[1] = ran_float(radius_1, -radius_1);
		p->Position[2] = ran_float(radius_1, -radius_1);

		glm::vec3 dis = p->Position - p_center;
		float dis_c = dot(dis, dis);

		p->Velocity[0] = ran_float(radius_1 / 20., -radius_1 / 20.);
		p->Velocity[1] = ran_float(radius_1 / 20. * 3., 0.);
		p->Velocity[2] = ran_float(radius_1 / 20., -radius_1 / 20.);

		p->Life = ran_int(il / 5, 0);

		if (dot(dis, dis) <= (radius_1 * radius_1))
			particles_ex.push_back(p);
		else
			free(p);
	}

	for (int i = 0; i < num_particles / 2; i++)
	{
		Particle* p = new Particle();

		p->Color[0] = 1.;
		p->Color[1] = 0.67;
		p->Color[2] = 0.;
		p->Color[3] = 1.;

		p->Position[0] = ran_float(radius_2, -radius_2);
		p->Position[1] = ran_float(radius_2, -radius_2);
		p->Position[2] = ran_float(radius_2, -radius_2);

		glm::vec3 dis = p->Position - p_center;
		float dis_c = dot(dis, dis);

		p->Velocity[0] = ran_float(radius_2 / 30., -radius_2 / 30.);
		p->Velocity[1] = ran_float(radius_2 / 30. * 3., 0.);
		p->Velocity[2] = ran_float(radius_2 / 30., -radius_2 / 30.);

		p->Life = ran_int(il / 3, 0);

		if (dot(dis, dis) <= (radius_2 * radius_2))
			particles_ex.push_back(p);
		else
			free(p);

	}
}

void display_particles_ex()
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < particles_ex.size(); i++)
	{
		glColor4f(particles_ex[i]->Color[0], particles_ex[i]->Color[1], particles_ex[i]->Color[2], particles_ex[i]->Color[3]);
		glVertex3f(particles_ex[i]->Position[0], particles_ex[i]->Position[1], particles_ex[i]->Position[2]);
	}
	glEnd();
}


void display_particles()
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < particles.size(); i++)
	{
		glColor4f(particles[i]->Color[0], particles[i]->Color[1], particles[i]->Color[2], particles[i]->Color[3]);
		glVertex3f(particles[i]->Position[0], particles[i]->Position[1], particles[i]->Position[2]);
	}
	glEnd();
}

void free_particles()
{
	for (int i = 0; i < num_particles; i++)
	{
		free(particles_ex[i]);
	}
}

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	srand(static_cast <unsigned> (time(0)));

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display structures that will not change:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never returns
	// this line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	// force a call to Display( ) next time it is convenient:
	int temp;
	temp = glutGet(GLUT_ELAPSED_TIME);
	elp = temp - last_elp;
	last_elp = temp;

	if (inAnimation)
	{

		tt = temp - elp_time;	// milliseconds

		if (tt >= il)
		{
			//glutIdleFunc(NULL);
			inAnimation = FALSE;
			tt = 0;
		}
		update_particles_ex();
	}
	update_particles();
	if (FirstPerson)
		glutWarpPointer(INIT_WINDOW_SIZE / 2, INIT_WINDOW_SIZE / 2);
	glutSetWindow(MainWindow);
	glutPostRedisplay();
	
}


// draw the complete scene:

void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_SMOOTH );


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );


	// set the eye position, look-at position, and up-vector:
	
	//lighting
	glEnable(GL_LIGHTING);
	glPushMatrix();
	SetPointLight(GL_LIGHT0, 0., 0., 3., 1., 1., 1.);
	glPopMatrix();

	if (!FirstPerson)
		gluLookAt( 0., 0., 3.,     0., 0., 0.,     0., 1., 0. );
	else 
		gluLookAt( 0., 0., 0.,	   1., 0., 0.,	   0., 1., 0.);


	// rotate the scene:
	if (FirstPerson)
		Camera();
	else
	{
		glRotatef((GLfloat)Yrot, 0., 1., 0.);
		glRotatef((GLfloat)Xrot, 1., 0., 0.);
	}
	

	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );


	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// since we are using glScalef( ), be sure normals get unitized:

	glEnable( GL_NORMALIZE );
	glEnable(GL_BLEND);

	// draw the current object:

	//animation
	float _t1 = 0.;
	float _t2 = 0.;
	float _t = (float)tt / (float)il;
	if (_t > 0.25 && _t < 0.75)
		_t1 = (_t - 0.25) * 2.;
	if (_t > 0.5)
		_t2 = (_t - 0.5) * 2.;

	//blind texture
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, Tex0);

	//top
	glPushMatrix();
	if (FirstPerson)
		FP_rotate();
	if (inAnimation)
		glTranslatef(-0.25 * sinf(3.1415926 * _t1), 0., 0.);
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glCallList( BoxList );
	glPopMatrix();

	//bottom
	glPushMatrix();
	if (FirstPerson)
		FP_rotate();
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glCallList( BoxList + 1 );
	glPopMatrix();

	//triger
	glPushMatrix();
	if (FirstPerson)
		FP_rotate();
	if (inAnimation)
		glTranslatef(-0.1 * sinf(3.1415926 * _t), 0., 0.);
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glCallList( BoxList + 2 );
	glPopMatrix();

	//mag
	glPushMatrix();
	if (FirstPerson)
		FP_rotate();
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glCallList( BoxList + 3 );
	glPopMatrix();


	//bullet body
	glBindTexture(GL_TEXTURE_2D, Tex1);
	glPushMatrix();
	if (FirstPerson)
		FP_rotate();
	if (inAnimation)
	{
		glTranslatef(0., 2. * sinf(3.1415926 * _t2), 5. * _t2);
	}
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glCallList( BoxList + 4 );
	glPopMatrix();

	//bullet head
	glBindTexture(GL_TEXTURE_2D, Tex2);
	glPushMatrix();
	if (FirstPerson)
		FP_rotate();
	if (inAnimation)
		glTranslatef(20. * _t2, 0., 0.);
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glCallList( BoxList + 5 );
	glPopMatrix();

	//cyl
	glPushMatrix();
	glColor3f(1., 1., 1.);
	if (FirstPerson)
		FP_rotate();
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glCallList(BoxList + 6);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	SetMaterial(1., 0., 0., 80);
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glTranslatef(15., 0., 0.);
	glCallList(BoxList + 7);
	glPopMatrix();

	glPushMatrix();
	SetMaterial(0., 1., 0., 80);
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glTranslatef(15., 0., -3.);
	glCallList(BoxList + 7);
	glPopMatrix();

	glPushMatrix();
	SetMaterial(0., 0., 1., 80);
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glTranslatef(15., 0., 3.);
	glCallList(BoxList + 7);
	glPopMatrix();

	glDisable(GL_LIGHTING);

	//particles
	glPushMatrix();
	if (FirstPerson)
		FP_rotate();
	if (FirstPerson)
		glTranslatef(2., -1., 0.);
	glTranslatef(0.85, 0.5, 0.);
	glRotatef(-90., 0., 0., 1.);
	if (inAnimation && _t2 > 0)
		display_particles_ex();
	glPopMatrix();

	if (showFire)
	{
		glPushMatrix();
		if (FirstPerson)
			glTranslatef(2., -1., 0.);
		glTranslatef(15., -1., 0.);
		glScalef(50., 50., 70.);
		display_particles();
		glPopMatrix();
	}

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.,   0., 1., 0. );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif

	// possibly draw the axes:

	if (AxesOn != 0)
	{
		glColor3fv(&Colors[WhichColor][0]);
		glCallList(AxesList);
	}


	// draw some gratuitous text that just rotates on top of the scene:

	//glDisable( GL_DEPTH_TEST );
	//glColor3f( 0., 1., 1. );
	//DoRasterString( 0., 1., 0., (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	//glDisable( GL_DEPTH_TEST );
	//glMatrixMode( GL_PROJECTION );
	//glLoadIdentity( );
	//gluOrtho2D( 0., 100.,     0., 100. );
	//glMatrixMode( GL_MODELVIEW );
	//glLoadIdentity( );
	//glColor3f( 1., 1., 1. );
	//DoRasterString( 5., 5., 0., (char *)"Text That Doesn't" );


	// swap the double-buffered framebuffers:

	glutSwapBuffers( );


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	WhichColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	WhichProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoShadowsMenu(int id)
{
	ShadowsOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int shadowsmenu = glutCreateMenu(DoShadowsMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Colors",        colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );

#ifdef ENABLE_SHADOWS
	glutAddSubMenu(   "Shadows",       shadowsmenu);
#endif

	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	glutIdleFunc( Animate );
	glutSetCursor(NULL);

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	//Texture
	Texture = BmpToTexture("./texture/gun.bmp", &width, &height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &Tex0);
	glBindTexture(GL_TEXTURE_2D, Tex0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);

	Texture1 = BmpToTexture("./texture/gold.bmp", &width1, &height1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &Tex1);
	glBindTexture(GL_TEXTURE_2D, Tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture1);

	Texture2 = BmpToTexture("./texture/silver.bmp", &width2, &height2);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &Tex2);
	glBindTexture(GL_TEXTURE_2D, Tex2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture2);

	//Transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{

	glutSetWindow( MainWindow );

	// create the object:

	BoxList = glGenLists( 8 );
	glNewList( BoxList, GL_COMPILE );

	LoadObjFile("top.obj");
		
	glEndList( );

	glNewList(BoxList+1, GL_COMPILE);

	LoadObjFile("bottom.obj");

	glEndList();

	glNewList(BoxList + 2, GL_COMPILE);

	LoadObjFile("triger.obj");

	glEndList();

	glNewList(BoxList + 3, GL_COMPILE);

	LoadObjFile("mag.obj");

	glEndList();

	glNewList(BoxList + 4, GL_COMPILE);

	glTranslatef(-0.45, 0.5, 0.);
	glScalef(0.6, 1., 1.);
	glTranslatef(0., 0., 0.16);
	glRotatef(-113., 0., 1., 0.);
	glScalef(0.1, 0.1, 0.1);
	LoadObjFile("bb.obj");

	glEndList();

	glNewList(BoxList + 5, GL_COMPILE);

	glTranslatef(0.2, 0.33, 0.);
	glScalef(0.6, 1., 1.);
	glRotatef(90., 0., 1., 0.);
	glScalef(0.1, 0.1, 0.1);
	LoadObjFile("bh.obj");

	glEndList();

	glNewList(BoxList + 6, GL_COMPILE);
	glScalef(1.4, 2.6, 2.6);
	glTranslatef(0.51, 0.19, 0.);
	glRotatef(90., 0., 1., 0.);
	glScalef(0.1, 0.1, 0.1);
	LoadObjFile("cyl.obj");

	glEndList();

	glNewList(BoxList + 7, GL_COMPILE);
	glTranslated(3., 0., 0.);
	MjbSphere(0.5, 20, 20);
	glEndList();

	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			WhichProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		case 'f':
		case 'F':
			FirstPerson = !FirstPerson;
			if (FirstPerson)
			{
				pitch = 0.;
				yaw = 0.;
				glutSetCursor(GLUT_CURSOR_NONE);
				glutPassiveMotionFunc(Passive_motion);
				AxesOn = FALSE;
			}
			else
			{
				Xrot = 0.;
				Yrot = 0.;
				glutSetCursor(NULL);
				glutPassiveMotionFunc(NULL);
			}
			break;

		case 'e':
		case 'E':
			showFire = !showFire;
			break;

		case ' ':
			if (!inAnimation)
			{
				//glutIdleFunc(Animate);
				inAnimation = TRUE;
				PlaySound("./gun.wav", NULL, SND_FILENAME | SND_ASYNC);
				elp_time = glutGet(GLUT_ELAPSED_TIME);	// milliseconds
				last_elp = glutGet(GLUT_ELAPSED_TIME);	// milliseconds

			}
			break;

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		
			if (!inAnimation && FirstPerson)
			{
				//glutIdleFunc(Animate);
				inAnimation = TRUE;
				PlaySound("./gun.wav", NULL, SND_FILENAME | SND_ASYNC);
				elp_time = glutGet(GLUT_ELAPSED_TIME);	// milliseconds
				last_elp = glutGet(GLUT_ELAPSED_TIME);	// milliseconds
			}
			break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0  && !FirstPerson)
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}

void Passive_motion(int x, int y)
{
	/* two variables to store X and Y coordinates, as observed from the center
	  of the window
	*/
	int dev_x, dev_y;
	dev_x = (INIT_WINDOW_SIZE / 2) - x;
	dev_y = (INIT_WINDOW_SIZE / 2) - y;

	/* apply the changes to pitch and yaw*/
	yaw += (float)dev_x / 10.0;
	pitch += (float)dev_y / 10.0;
}

void Camera()
{
	/*limit the values of pitch
	  between -60 and 70
	*/
	if (pitch >= 70)
		pitch = 70;
	if (pitch <= -60)
		pitch = -60;
	glRotatef(-pitch, 0.0, 0.0, 1.0); // Along Z axis
	glRotatef(-yaw, 0.0, 1.0, 0.0);    //Along Y axis
}

void FP_rotate()
{
	glRotatef(pitch, 0.0, 0.0, 1.0); // Along Z axis
	glRotatef(yaw, 0.0, 1.0, 0.0);    //Along Y axis
}
///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}

struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

// read a BMP file into a Texture:

unsigned char *
BmpToTexture( char *filename, int *width, int *height )
{
	FILE *fp = fopen( filename, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
	}

	FileHeader.bfType = ReadShort( fp );


	// if bfType is not 0x4d42, the file is not a bmp:

	if( FileHeader.bfType != 0x4d42 )
	{
		fprintf( stderr, "File '%s' is the wrong type of file: 0x%0x\n", filename, FileHeader.bfType );
		fclose( fp );
		return NULL;
	}

	FileHeader.bfSize = ReadInt( fp );
	FileHeader.bfReserved1 = ReadShort( fp );
	FileHeader.bfReserved2 = ReadShort( fp );
	FileHeader.bfOffBits = ReadInt( fp );

	InfoHeader.biSize = ReadInt( fp );
	InfoHeader.biWidth = ReadInt( fp );
	InfoHeader.biHeight = ReadInt( fp );

	int nums = InfoHeader.biWidth;
	int numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort( fp );
	InfoHeader.biBitCount = ReadShort( fp );
	InfoHeader.biCompression = ReadInt( fp );
	InfoHeader.biSizeImage = ReadInt( fp );
	InfoHeader.biXPelsPerMeter = ReadInt( fp );
	InfoHeader.biYPelsPerMeter = ReadInt( fp );
	InfoHeader.biClrUsed = ReadInt( fp );
	InfoHeader.biClrImportant = ReadInt( fp );

	fprintf( stderr, "Image size in file '%s' is: %d x %d\n", filename, nums, numt );

	unsigned char * texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\b" );
		return NULL;
	}

	// extra padding bytes:

	int numextra =  4*(( (3*InfoHeader.biWidth)+3)/4) - 3*InfoHeader.biWidth;

	// we do not support compression:

	if( InfoHeader.biCompression != birgb )
	{
		fprintf( stderr, "Image file '%s' has the wrong type of image compression: %d\n", filename, InfoHeader.biCompression );
		fclose( fp );
		return NULL;
	}

	rewind( fp );
	fseek( fp, 14+40, SEEK_SET );

	if( InfoHeader.biBitCount == 24 )
	{
		unsigned char *tp = texture;
		for( int t = 0; t < numt; t++ )
		{
			for( int s = 0; s < nums; s++, tp += 3 )
			{
				*(tp+2) = fgetc( fp );		// b
				*(tp+1) = fgetc( fp );		// g
				*(tp+0) = fgetc( fp );		// r
			}

			for( int e = 0; e < numextra; e++ )
			{
				fgetc( fp );
			}
		}
	}

	fclose( fp );

	*width = nums;
	*height = numt;
	return texture;
}

int
ReadInt( FILE *fp )
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	b2 = fgetc( fp );
	b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}

short
ReadShort( FILE *fp )
{
	unsigned char b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

/*
void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}
*/

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

/*
float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}
*/