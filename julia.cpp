#ifdef _WIN32
#include <GL/glut.h>
#include <GL/glu.h>
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glut32.lib")
#elif __APPLE__
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <math.h>

const static int max_iteration = 8;
//==============================================================================
class Complex
//==============================================================================
{
public:
  float re, im;

  Complex( float re = 0, float im = 0 ) : re(re), im(im) {}
};

Complex operator+( const Complex& c1, const Complex& c2 )
{
  return Complex( c1.re + c2.re, c1.im + c2.im );
}

Complex operator*( const Complex& c1, const Complex& c2 )
{
  return Complex( c1.re*c2.re - c1.im*c2.im, c1.re*c2.im + c1.im*c2.re );
}


//==============================================================================
class Extent
//==============================================================================
{
public:
  float l,r,b,t;
  Extent(float l = -1, float r = 1, float b = -1, float t = 1 ) :
    l(l), r(r), b(b), t(t) {}
};


// glut callbacks
void display();
void keyboard(unsigned char k, int x, int y);
void mouse( int button, int state, int x, int y );
void reshape( int w, int h );
void idle();

Extent world(-1,1,-1,1);
Complex c(0.109, 0.603);
int width = 512, height = 512;
bool doJuliaSet = true;

//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize( width, height );  
  glutCreateWindow("Fractal Land");
  glutDisplayFunc( display );
  glutMouseFunc( mouse );
  glutKeyboardFunc( keyboard );  
  glutReshapeFunc( reshape );
  glutMainLoop();

	return 0;
}

//------------------------------------------------------------------------------
void julia( Complex p, Complex c, int& i, float& r )
{
  float rSqr;
  int maxIterations = max_iteration;
  for( i=0; i < maxIterations; i++ )
  {
    p = p*p + c;
    rSqr = p.re*p.re + p.im*p.im;
    if( rSqr > 4 )
      break;
  }
  r = sqrt(rSqr);
}

//------------------------------------------------------------------------------
void mandelbrot( Complex c, int& i, float& r )
{
  float rSqr;
  int maxIterations = max_iteration;
  Complex p(0,0);
  for( i=0; i < maxIterations; i++ )
  {
    p = p*p + c;
    rSqr = p.re*p.re + p.im*p.im;
    if( rSqr > 4 )
      break;
  }
  r = sqrt(rSqr);
}

//-----------------------------------------------------------------------------
void display()
{
  // Clear the screen
  glClearColor(0,0,1,0);
  glClear(GL_COLOR_BUFFER_BIT);

  // Setup the viewing matrices
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(world.l, world.r, world.b, world.t);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // loop over the pixels on the screen
  float delta = (world.r - world.l)/float(width);
  for( int j=0; j < height; j++ )
  {      
      for( int i=0; i < width; i++ )
      {
        // convert pixel location to world coordinates
        float x = world.l + i*delta;
        float y = world.b + j*delta;      
        
        // test for convergence
        int its;
        float R;
        Complex p(x,y);
        if( doJuliaSet )
          julia( p, c, its, R );
        else
          mandelbrot( p, its, R );

        // turn iterations and radius to color
        if( its == max_iteration )
          glColor3d(0,0,0);
        else
        {
          float r = R/float(3);
          float g = its/float(128);
          float b = R/float(its+1);
          glColor3d(r,g,b);
        }

        // draw polygon
        glBegin(GL_POLYGON);
        glVertex2d(x, y);
        glVertex2d(x, y+delta);
        glVertex2d(x+delta, y+delta);
        glVertex2d(x+delta, y);
        glEnd();
      }
    glFlush();
  }  
}


//-----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
  if ((key == 'r') || (key == 'R'))
  {
    if( doJuliaSet )
    {
      // return to initial position
      c = Complex(0.109, 0.603);
      world.l = -1;
      world.r = 1;
      world.b = -1;
      world.t = 1;
    }
    else
    {
      world.l = -2;
      world.r = 2;
      world.b = -2;
      world.t = 2;
    }
    display();
  }
  else if((key == 'c') || (key == 'C') )
  {
    // set c = (0,0)
    c = Complex(0, 0);
    world.l = -1;
    world.r = 1;
    world.b = -1;
    world.t = 1;
    display();
  }
  else if( key == ' ' )
  {
    doJuliaSet = !doJuliaSet;
    display();
  }
}

//------------------------------------------------------------------------------
float xScreenToWorld(float scrX)
{
  return ((world.r - world.l) * scrX / float(width)) + world.l;
}

//------------------------------------------------------------------------------
float yScreenToWorld(float scrY)
{
  return ((world.t - world.b) * (1 - scrY / float(height))) + world.b;
}


//-----------------------------------------------------------------------------
void mouse( int button, int state, int mx, int my )
{
  float x = xScreenToWorld(mx);
  float y = yScreenToWorld(my);
  float dx = (world.r - world.l);  
  float dy = (world.t - world.b);
  if( (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN) )
  {
    world.l = x - dx/4;
    world.r = x + dx/4;
    world.b = y - dy/4;
    world.t = y + dy/4;
    display();
  }
  else if( (button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN) )
  {
    world.l = x - dx;
    world.r = x + dx;
    world.b = y - dy;
    world.t = y + dy;
    display();
  }
}

//------------------------------------------------------------------------------
void reshape( int w, int h)
{
//  width = w;
//  height = h;
  glViewport(0, 0, w, h);

 
//  float cx = 0.5*(world.r + world.l);
//  float dy = world.t - world.b;;
//  world.l = cx - 0.5*dy * w/h;
//  world.r = cx + 0.5*dy * w/h;
}
