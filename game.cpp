#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include "SOIL.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)

using namespace std;

struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;

  GLenum PrimitiveMode;
  GLenum FillMode;
  int NumVertices;
};
typedef struct VAO VAO;

struct bb{
  float x;
  float y;
  float vx;
  float vy;
  float speedY;
  float speedX;
  float radius;
  float angle;
  float fixed;
  float dead;
  float prev;
  float cur;
};

typedef struct bb bird;

typedef struct xx
{
  VAO *thing;
  float x;
  float y;
  float width;
  float height;
}OBJ;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
} Matrices;

GLuint programID;

float zoom = 1.0f;
int pan = 1;

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
void createBall(float rad) ;
void quit(GLFWwindow *window);

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
  }

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
  struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
  {
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
      color_buffer_data [3*i] = red;
      color_buffer_data [3*i + 1] = green;
      color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
  }

/* Render the VBOs handled by VAO */
  void draw3DObject (struct VAO* vao);


  float rectangle_rot_dir = 1;
  bool rectangle_rot_status = true;

  float grass_rot_dir = 1;
  bool grass_rot_status = false;


void reshapeWindow (GLFWwindow* window, int width, int height)
{
  int fbwidth=width, fbheight=height;

  glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 90.0f;

  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

  Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);
  /*
  Matrices.projection = glm::ortho( (zoom*(-100.0f))+pan, (zoom*(100.0f))+pan, zoom*(-100.0f), zoom*(100.0f), 0.1f, 500.0f);
  */
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



VAO *rectangle, *Circle, *grass, *xAxis, *yAxis, *cannon, *cannonBase, *object1, *object2, *object3, *object4, *object5, *object6, *sky, *powerBar;

OBJ circles[10];

OBJ life[10];

VAO *objectArray[4];

bird birds[5];

int current;

int counter;

int collected_x = 60;

int collected_y = 80;

int mouse_active = 0;

int xBar = -10;

float base;

int fall_flag;

int iscollide;

float scrolled = 0.0f;

float base_power = 1.5;

float cannon_angle = 0.0f;

float newAngle = 0.0f;

float control_angle = 0.0f;

float projectile_angle = 0.0f;

float gravitas=0.58;

float grass_width = 100, grass_x = -70, grass_y = -70;

float camera_rotation_angle = 0, rectangle_rotation = 0, currtime,mass_circle=0.7;

OBJ rects[10]; // Stores the objects

void updateAngle(bird *newBird, int dir);
void initializeBirds(bird *newBird, float x, float y, float vx, float vy);

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
  {
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
      switch (key) {
        case GLFW_KEY_C:
        rectangle_rot_status = !rectangle_rot_status;
        break;
        case GLFW_KEY_A:
                          break;
        default:
                          break;
      }
    }

    if(action == GLFW_REPEAT)
    {
     switch (key) {
      case GLFW_KEY_A: cannon_angle+=1;
      control_angle+=1.5;
                      break;
      case GLFW_KEY_B: cannon_angle-=1;
      control_angle-=1.5;
                      break;
      case GLFW_KEY_F:gravitas+=0.005;
                      base_power+=0.02;
                      xBar += 3;
      break;

      case GLFW_KEY_S:gravitas-=0.005;
                      base_power-=0.02;
                      xBar -=3;
      break;
      case GLFW_KEY_DOWN:
      if(zoom < 1)
        zoom += 0.01f;
      break;

      case GLFW_KEY_UP:
      if(zoom >=0.8)
        zoom -= 0.01f;
      break;
      default:                
                      break;
    }
  }

  else if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE: quit(window);
                            break;
      case GLFW_KEY_SPACE: projectile_angle = control_angle;
                       cout<<projectile_angle<<endl;
                       updateAngle(&birds[current],1);
                       birds[current].fixed = 1; 
                      break;
      case GLFW_KEY_R: if(current<4)
                       {
                          current = current+1;
                          base_power = 1.5;
                          gravitas = 0.58;
                          initializeBirds(&birds[current], -70, -63, 0, 0);
                          updateAngle(&birds[current],1);
                          fall_flag=0;
                          iscollide=0;
                          xBar = -10;
                       }
                       else
                          initializeBirds(&birds[current], -10000000, -63, 0, 0);                        
                       break;

      case GLFW_KEY_F:gravitas+=0.005;
                      base_power+=0.02;
                      xBar += 3;
      break;

      case GLFW_KEY_S:gravitas-=0.005;
                      base_power-=0.02;
                      xBar -=3;
      break;

      case GLFW_KEY_DOWN:
      pan = 0;
      if(zoom < 1)
        zoom += 0.01f;
      break;

      case GLFW_KEY_UP:
      pan = 0;
      if(zoom >=0.8)
        zoom -= 0.01f;
      break;

      case GLFW_KEY_LEFT:
      if(-(zoom*100)+pan > -100)
        pan-=1;
      break;

      case GLFW_KEY_RIGHT:
      if((zoom*100)+pan < 100)
        pan+=1;
      break;

      case GLFW_KEY_A: cannon_angle+=1;
                      control_angle+=1.5;
                      break;
      case GLFW_KEY_B: cannon_angle-=1;
                      control_angle-=1.5;
                      break;
      default:
      break;
    }
  }
}


/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
  switch (key) {
    case 'Q':
    case 'q':
    quit(window);
    break;
    default:
    break;
  }
}

void returnMouse(GLFWwindow *window, double xpos, double ypos)
{
    xpos = (xpos-350)/3.5;
    ypos = (350-ypos)/3.5;
    float xclick = xpos;
    float yclick = ypos;
    glm::vec2 pointer(xpos,ypos);
    glm::vec2 cannon(-70,-63);
    glm::vec2 di = pointer - cannon;
    glm::vec2 axis(1,0);
    cout<<base_power<<endl;
    float dota3 = glm::dot(di,axis);
    float cosine = dota3 / glm::length(di);
    newAngle = acos(cosine)*180.0f/M_PI;

/*
    if(ypos<-232)
    {
  cangle = -cangle;
    }
    cangle -= 90;
*/
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      cout<<"MOUSE CLICK : RIGHT"<<endl;
      if (action == GLFW_PRESS){
        mouse_active = 1;
        cannon_angle = newAngle/1.5;
        control_angle = newAngle;
      }
      else if (action == GLFW_RELEASE) {
        mouse_active = 0;
        cannon_angle = newAngle/1.5;
        control_angle = newAngle;
      }
      break;

    default:
      break;
  }
}

void returnScroll(GLFWwindow * window, double horizontal, double vertical)
{
  cout<<xBar<<endl;
    scrolled = vertical;
    if(scrolled == -1)
      if(zoom < 1)
        zoom += 0.01f;
      
    if(scrolled == 1)  
      if(zoom >=0.8)
        zoom -= 0.01f;
}

// Function to initialize individial birds
void initializeBirds(bird *newBird, float x, float y, float vx, float vy)
{
  newBird->x = x;
  newBird->y = y;
  newBird->vx = vx;
  newBird->vy = vy;
  newBird->speedX = 0.31;
  newBird->speedY = 0.31;
  newBird->radius = 1.3;
  newBird->angle = 45;
  newBird->fixed = 0;
  newBird->dead = 0;
  newBird->prev = 0;
  newBird->cur = 0;
}

void updateAngle(bird *newBird, int dir)
{
    newBird->speedY = base_power*sin(DEG2RAD(projectile_angle));
    newBird->speedX = base_power*cos(DEG2RAD(projectile_angle));
    cout << control_angle << endl;
    cout << newBird->speedX << endl;
    cout << newBird->speedY << endl;
}

void createGrass ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -100,-100,0, //  1
    100,-100,0, //  2
    -100, -70,0, //  3

    100,-100,0, //  2
    -100, -70,0, //  3
    100, -70, 0     // 4
  };

  static const GLfloat color_buffer_data [] = {
    0,0.7,0, // color 1
    0,0.7,0, // color 2
    0,0.7,0, // color 3

    0,0.8,0, // color 3
    0,0.8,0, // color 4
    0,0.8,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  grass = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createSky ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -100,-100,0, //  1
    100,-100,0, //  2
    -100, 100,0, //  3

    100,-100,0, //  2
    -100, 100,0, //  3
    100, 100, 0     // 4
  };

  static const GLfloat color_buffer_data [] = {
    0.325,0.627,0.929, // color 1
    0.325,0.627,0.929, // color 2
    0.325,0.627,0.929, // color 3

    0.325,0.627,0.929, // color 3
    0.325,0.627,0.929, // color 4
    0.325,0.627,0.929  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  sky = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createXAxis ()
{

  static const GLfloat vertex_buffer_data [] = {
   -100,0.2,0, // vertex 1
   -100,-0.2,0, // vertex 2
    100,-0.2,0, // vertex 3

    100,-0.2,0, // vertex 3
    100, 0.2,0, // vertex 4
   -100, 0.2,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  xAxis = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createPowerBar ()
{

  static const GLfloat vertex_buffer_data [] = {
   -300,100,0, // vertex 1
   -300,95,0, // vertex 2
    -90,95,0, // vertex 3

    -90,95,0, // vertex 3
    -90, 100,0, // vertex 4
   -300, 100,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  powerBar = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createYAxis ()
{

  static const GLfloat vertex_buffer_data [] = {
   0.2,-100,0, // vertex 1
   -0.2,-100,0, // vertex 2
    -0.2,100,0, // vertex 3

   -0.2,100,0, // vertex 3
    0.2,100, 0, // vertex 4
   0.2,-100, 0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  yAxis = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBall(float rad)
{

  rad = 1.5;

  int i,k=0;

  GLfloat vertex_buffer_data[1090]={};
  GLfloat color_buffer_data[1090]={};
  vertex_buffer_data[k++]=0;
  vertex_buffer_data[k++]=0;
  vertex_buffer_data[k++]=0;

  for (i = 1; i < 361; ++i)
  {
   vertex_buffer_data[k] = rad*cos(DEG2RAD(i));
   color_buffer_data[k] = 1;
       // cout << ((double) rand() / (RAND_MAX)) << endl;

   k++;
   vertex_buffer_data[k] = rad*sin(DEG2RAD(i));
   color_buffer_data[k] = 1;
   k++;
   vertex_buffer_data[k] = 0;
   color_buffer_data[k] = 1;
   k++;
 }

 Circle = create3DObject(GL_TRIANGLE_FAN, 361, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCannonBase()
{
  float rad = 5;

  int i,k=0;

  GLfloat vertex_buffer_data[1090]={};
  GLfloat color_buffer_data[1090]={};
  vertex_buffer_data[k++]=0;
  vertex_buffer_data[k++]=0;
  vertex_buffer_data[k++]=0;

  for (i = 1; i < 361; ++i)
  {
   vertex_buffer_data[k] = rad*cos(DEG2RAD(i));
   color_buffer_data[k] = 1;
       // cout << ((double) rand() / (RAND_MAX)) << endl;

   k++;
   vertex_buffer_data[k] = rad*sin(DEG2RAD(i));
   color_buffer_data[k] = 1;
   k++;
   vertex_buffer_data[k] = 0;
   color_buffer_data[k] = 1;
   k++;
 }

 cannonBase = create3DObject(GL_TRIANGLE_FAN, 361, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCannon ()
{

  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    20,0,0, // vertex 2
    20,-5,0, // vertex 3

    20,-5,0, // vertex 3
    0,-5,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };
  cannon = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createObject1()
{

  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    20,0,0, // vertex 2
    20,-5,0, // vertex 3

    20,-5,0, // vertex 3
    0,-5,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };
  object1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createObject2()
{

  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    20,0,0, // vertex 2
    20,-5,0, // vertex 3

    20,-5,0, // vertex 3
    0,-5,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };
  object2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createObject3()
{

  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    20,0,0, // vertex 2
    20,-5,0, // vertex 3

    20,-5,0, // vertex 3
    0,-5,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };
  object3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createObject4()
{

  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    10,0,0, // vertex 2
    10,-10,0, // vertex 3

    10,-10,0, // vertex 3
    0,-10,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,1,0, // color 1
    1,1,0, // color 2
    1,1,0, // color 3

    1,1,0, // color 3
    1,1,0, // color 4
    1,1,0  // color 1
  };
  object4 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createObject5()
{

  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    10,0,0, // vertex 2
    10,-10,0, // vertex 3

    10,-10,0, // vertex 3
    0,-10,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,1,0, // color 1
    1,1,0, // color 2
    1,1,0, // color 3

    1,1,0, // color 3
    1,1,0, // color 4
    1,1,0  // color 1
  };
  object5 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createObject6()
{

  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    10,0,0, // vertex 2
    10,-10,0, // vertex 3

    10,-10,0, // vertex 3
    0,-10,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1
  };
  object6 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}



void projectile(bird *newBird)
{

  currtime = glfwGetTime();  
  
  if(newBird->y <= -110.0f);

  else if (newBird->fixed != 0)
  {
    
    newBird->dead+= 0.035f;              // Timer
    newBird->vy = newBird->speedY - (0.8)*(newBird->dead*gravitas); // v = u - gt
    newBird->vx = newBird->speedX;      // v = u

    newBird->x += newBird->vx;          //
    newBird->y += newBird->vy;


   /* newBird->dead += 0.01f;
    newBird->vy = newBird->speedY*newBird->dead - (newBird->dead)*newBird->dead*(0.15);
    newBird->vx = newBird->speedX*newBird->dead;
    newBird->x = -70 + newBird->vx;
    newBird->y = -63 + newBird->vy ;
  */
  }
}


VAO* createCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides, float start_angle, float end_angle, float red_value, float green_value, float blue_value, float red_center, float green_center, float blue_center)
{
    int numberOfVertices = (numberOfSides + 2)*abs(start_angle - end_angle)/360,i;

    GLfloat twicePi = 2.0f * M_PI;

    GLfloat circleVerticesX[numberOfVertices];
    GLfloat circleVerticesY[numberOfVertices];
    GLfloat circleVerticesZ[numberOfVertices];

    circleVerticesX[0] = x;
    circleVerticesY[0] = y;
    circleVerticesZ[0] = z;

    for ( i = 1; i < numberOfVertices; i++ )
    {
        circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides + (start_angle*M_PI/360)) );
        circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides + (start_angle*M_PI/360)) );
        circleVerticesZ[i] = z;
    }

    GLfloat allCircleVertices[( numberOfVertices ) * 3];
    GLfloat allCircleColors[( numberOfVertices ) * 3];

    for ( int i = 0; i < numberOfVertices; i++ )
    {
        allCircleVertices[i * 3] = circleVerticesX[i];
        allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
        allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
    }
    
    for (int j=0;j<numberOfVertices*3;j++)
    {
        if(j%3==0)
            allCircleColors[j]=red_value;
        else if(j%3==1) 
            allCircleColors[j]=green_value;
        else
            allCircleColors[j] = blue_value;
    }
    allCircleColors[0] = red_center;
    allCircleColors[1] = blue_center;
    allCircleColors[2] = green_center;
    return create3DObject(GL_TRIANGLE_FAN,numberOfVertices, allCircleVertices, allCircleColors, GL_FILL);
}

void CoinCollision(bird *newBird, OBJ *object) // AABB - Circle collision
{
    // Get center point circle first 
  glm::vec2 center(newBird->x,newBird->y);

  glm::vec2 coin_center(object->x, object->y);
    // Get difference vector between both centers
  glm::vec2 difference = center - coin_center;

  // Add clamped value to AABB_center and we get the value of box closest to circle
    // Retrieve vector between center circle and closest point AABB and check if length <= radius

  if( glm::length(difference) < newBird->radius+5.1)
  { 

/*
    cout<<"andar"<<endl;

    if(newBird->prev == 0)
    {
      cout<<"collision"<<endl;
      iscollide = 1;
      newBird->cur = 1;
    }

    else
    {
      cout<<"no"<<endl;
      iscollide = 0;
      newBird->cur = 0;
    }
*/
    if(object->width != 4)
    {
      cout<<"CoinCollision"<<endl;

      collected_x += 4;

      object->x = collected_x;
      object->y = collected_y;
      object->width = 1.5;
    }
    else
      current = 0;
  }
}

void CheckCollision(bird *newBird, OBJ *object) // AABB - Circle collision
{
    // Get center point circle first 
  glm::vec2 center(newBird->x,newBird->y);
    // Calculate AABB info (center, half-extents)
  glm::vec2 aabb_half_extents(object->width / 2, object->height / 2);

  glm::vec2 aabb_center(object->x, object->y);
    // Get difference vector between both centers
  glm::vec2 difference = center - aabb_center;

  glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // Add clamped value to AABB_center and we get the value of box closest to circle
  glm::vec2 closest = aabb_center + clamped;
    // Retrieve vector between center circle and closest point AABB and check if length <= radius
  difference = closest - center;

  if( glm::length(difference) < newBird->radius)
  { 

/*
    cout<<"andar"<<endl;

    if(newBird->prev == 0)
    {
      cout<<"collision"<<endl;
      iscollide = 1;
      newBird->cur = 1;
    }

    else
    {
      cout<<"no"<<endl;
      iscollide = 0;
      newBird->cur = 0;
    }
*/

    iscollide = 1;

    currtime = glfwGetTime();

    glm::vec2 dotvector = center - closest;

    glm::vec2 yaxis(0.0,1.0);
    glm::vec2 xaxis(1.0,0.0);

    float dotproductx = glm::dot(dotvector,xaxis);
    float dotproducty = glm::dot(dotvector,yaxis);

    if(dotproducty == 0 && iscollide == 1)
    {
      cout<<"Hit Y-Axix"<<endl;

      cout<<newBird->x<<endl;
      cout<<newBird->y<<endl;

      newBird->prev = newBird->cur;
      newBird->cur = newBird->x;
      newBird->speedX /= 1.2;
      newBird->speedX = -newBird->speedX;

      if(newBird->speedX < 0)
      {
        newBird->x = object->x - object->width;
      }
      else
      {
        newBird->x = object->x + object->width;
      }

    }

    if(dotproductx == 0 && iscollide == 1)
    {

      if(cannon_angle <10)
      {
        newBird->speedY = -(newBird->vy);
      }
      else if(newBird->speedY < 0.2f && newBird->speedY > -0.1f)
      {
        newBird->y = -68.7;
        newBird->fixed = 0;
      }
      else;

      cout<<"Hit X-Axis"<<endl;
      newBird->speedX = newBird->speedX/1.4;
      newBird->speedY = -(newBird->vy/1.2);
      newBird->dead = 0;

      if(object->x <= 0.0)            // Collide with the grass
      {
        cout<<newBird->speedY<<endl;
        newBird->y = -68;
      }

      if(newBird->vy > 0 && object->x != 0)       // Collide with something else
        newBird->y -=2;
      else
        if(object->x != 0)
          newBird->y +=2;

    }

    if(object->width == 10.0f)
    {
      cout<<"Golden"<<endl;
      object->x = 100000;
      object->y = 100000;
    }
  }
  
  else
    iscollide =0 ;
}  


void Update()
{

  if(mouse_active == 1)
  {
    cannon_angle = newAngle/1.5;
    control_angle = newAngle;
  }

  for(int i=0;i<7;i++)
  {
    currtime = glfwGetTime();
    CheckCollision(&birds[current],&rects[i]);
  }
  for(int i=0; i<9; i++)
    CoinCollision(&birds[current],&circles[i]);
}



void draw ()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram (programID);

  glm::vec3 eye ( 100*cos(camera_rotation_angle*M_PI/180.0f), 100, 100*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  Matrices.view = glm::lookAt(eye, target, up);       // Rotating 3D Camera

/* Removed for 3-D
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
*/
  glm::mat4 VP = Matrices.projection * Matrices.view;

  glm::mat4 MVP;  // MVP = Projection * View * Model

  // Load identity to model matrix
  // Matrices.model = glm::mat4(1.0f);


  // glm::mat4 translateTriangle = glm::translate (glm::vec3(x_triangle, 0.0f, 0.0f)); // glTranslatef
  // glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  // glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  // Matrices.model *= triangleTransform; 
  // MVP = VP * Matrices.model; // MVP = p * V * M
  // glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  // draw3DObject(triangle)


  ////////////////////////// SKY ////////////////////////////////
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(sky);


  ///////////////////////////GRASS///////////////////////////
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(grass);

  ////////////////////////// X AXIS /////////////////////////
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(xAxis);

  ///////////////////////// Y AXIS //////////////////////////
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(yAxis);


  //////////////////////// CIRCLE  /////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCircle = glm::translate (glm::vec3(birds[current].x, birds[current].y, 0));        // glTranslatef
  Matrices.model *= translateCircle;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(Circle); 
  projectile(&birds[current]);


  ///////////////////// CANNON BODY /////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 rotateCannon = glm::rotate((float)(((cannon_angle))*M_PI/120.0f), glm::vec3(0,0,1));
  glm::mat4 translateCannon = glm::translate (glm::vec3(-70, -60, 0));        // glTranslatef
  Matrices.model *= (translateCannon * rotateCannon); // Rotate then Translate
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cannon);


  ////////////////////// CANON BASE ////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCannonBase = glm::translate (glm::vec3(-70, -63, 0));        // glTranslatef
  Matrices.model *= translateCannonBase;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cannonBase);

  ////////////////////   OBJECT 1 ////////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 rotateObj1 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 object1Base = glm::translate (glm::vec3(rects[1].x-2.5, rects[1].y-10, 0));        // glTranslatef
  Matrices.model *= (object1Base*rotateObj1);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(object1);

  ////////////////////   OBJECT 2 ////////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 rotateObj2 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 object2Base = glm::translate (glm::vec3(rects[2].x-2.5, rects[2].y-10, 0));        // glTranslatef
  Matrices.model *= (object2Base*rotateObj2);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(object2);

  ////////////////////   OBJECT 3 ////////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 rotateObj3 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 object3Base = glm::translate (glm::vec3(rects[3].x-2.5, rects[3].y-10, 0));        // glTranslatef
  Matrices.model *= (object3Base*rotateObj3);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(object3);

  ////////////////////   OBJECT 4 ////////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 rotateObj4 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 object4Base = glm::translate (glm::vec3(rects[4].x-5, rects[4].y-5, 0));        // glTranslatef
  Matrices.model *= (object4Base*rotateObj4);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(object4);

  ////////////////////   OBJECT 5 ////////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 rotateObj5 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 object5Base = glm::translate (glm::vec3(rects[5].x-5, rects[5].y-5, 0));        // glTranslatef
  Matrices.model *= (object5Base*rotateObj5);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(object5);

  ////////////////////   OBJECT 6 ////////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 rotateObj6 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 object6Base = glm::translate (glm::vec3(rects[6].x-5, rects[6].y-5, 0));        // glTranslatef
  Matrices.model *= (object6Base*rotateObj6);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(object6);

  /////////////////////// Power BAR ///////////////////////////
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBar = glm::translate (glm::vec3(xBar, 0  , 0));        // glTranslatef
  Matrices.model *= (translateBar); // Rotate then Translate
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(powerBar);


  int j = 0;
  for(int i=0; i<4; i++)
  {
    j++;
    j = j%2;
    if(collected_x == 60)
    {
      circles[i].width = 5;
      circles[i].x = 22*(i-4);
      circles[i].y = 20*j;
    }
    circles[i].thing = createCircle(circles[i].x,circles[i].y,0,circles[i].width,360,0,360,1,1,0,1,0,1);
    Matrices.model = glm::mat4(1.0f);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circles[i].thing);
  }

  j = 0;
  for(int i=4; i<6; i++)
  {
    j++;
    j = j%2;
    if(collected_x == 60)
    {
      circles[i].width = 5;
      circles[i].x = 20*(i-3);
      circles[i].y = 20*(i-3);
    }
    circles[i].thing = createCircle(circles[i].x,circles[i].y,0,circles[i].width,360,0,360,1,1,0,1,0,1);
    Matrices.model = glm::mat4(1.0f);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circles[i].thing);
  }

  for(int i=6; i<8; i++)
  {
    if(collected_x == 60)
    {
      circles[i].width = 5;
      circles[i].x = 20*(i-3);
      circles[i].y = 20*(8-i);
    }
    circles[i].thing = createCircle(circles[i].x,circles[i].y,0,circles[i].width,360,0,360,1,1,0,1,0,1);
    Matrices.model = glm::mat4(1.0f);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circles[i].thing);
  }

  circles[8].width = 4;
  circles[8].x = 45;
  circles[8].y = -12;
  circles[8].thing = createCircle(circles[8].x,circles[8].y,0,circles[8].width,360,0,360,rand()%2,rand()%2,rand()%2,1,1,1);
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(circles[8].thing);

  for(int i=0; i<4-current; i++)
  {
    life[i].width = 1.8;
    life[i].x = -90+(i*4.8);
    life[i].y = 80;
    life[i].thing = createCircle(life[i].x,life[i].y,0,life[i].width,360,0,360,1,0,1,1,1,0);
    Matrices.model = glm::mat4(1.0f);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(life[i].thing);
  }


  Update(); 
}


GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
      exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Angry Birds", NULL, NULL);

    if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    glfwSetWindowCloseCallback(window, quit);

    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetCursorPosCallback(window, returnMouse);
    glfwSetScrollCallback (window, returnScroll);


    return window;
  }

  void initGL (GLFWwindow* window, int width, int height)
  {
  // Generate the VAO, VBOs, vertices data & copy into the array buffer
  createGrass();
  createXAxis();
  createYAxis();
  createCannonBase();
  createCannon();
  createObject1();
  createObject2();
  createObject3();
  createObject4();
  createObject5();
  createObject6();
  createSky();
  createPowerBar();
  createBall(birds[current].radius);


  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


  reshapeWindow (window, width, height);

  glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
  glClearDepth (1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

  cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
  cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
  cout << "VERSION: " << glGetString(GL_VERSION) << endl;
  cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void makeBirds()
{
  for(int i=0; i<4; i++)
  {
    initializeBirds(&birds[i], -70, -63, 0, 0);

    current = 0 ; // Insert loop here 

    cout << birds[i].x << endl;
    cout << birds[i].y << endl;
    cout << birds[i].radius << endl;
    cout << birds[i].angle << endl;
  }
}


int main (int argc, char** argv)
{

  fall_flag = 0;
  counter = 0;
  iscollide = 0;

  objectArray[0] = rectangle;
  objectArray[1] = object1;
  objectArray[2] = object2;
  objectArray[3] = object3;
  objectArray[4] = object4;
  objectArray[5] = object5;
  objectArray[6] = object6;

  int width = 700;
  int height = 700;

  GLFWwindow* window = initGLFW(width, height);

  initGL (window, width, height);

  double last_update_time = glfwGetTime(), current_time;

/* Create And initialize the birds */

  makeBirds();

/* Adding things to the rects array */
  rects[0].thing = grass;
  rects[0].x = 0.0f;          // Centre
  rects[0].y = -85.0f;        // Centre
  rects[0].width = 200.0f;
  rects[0].height = 30.0f;

  rects[1].thing = object1;
  rects[1].x = 12.5f;
  rects[1].y = -10.0f;
  rects[1].width = 5.0f;
  rects[1].height = 20.0f;

  rects[2].thing = object2;
  rects[2].x = 45.0f;
  rects[2].y = -50.0f;
  rects[2].width = 5.0f;
  rects[2].height = 20.0f;

  rects[3].thing = object3;
  rects[3].x = 77.5f;
  rects[3].y = -10.0f;
  rects[3].width = 5.0f;
  rects[3].height = 20.0f;

  rects[4].thing = object4;
  rects[4].x = 60;
  rects[4].y = -20;
  rects[4].width = 10.0f;
  rects[4].height = 10.0f;

  rects[5].thing = object5;
  rects[5].x = 30;
  rects[5].y = -20;
  rects[5].width = 10.0f;
  rects[5].height = 10.0f;

  rects[6].thing = object6;
  rects[6].x = 45;
  rects[6].y = 0;
  rects[6].width = 10.0f;
  rects[6].height = 10.0f;

    /* Draw in loop */
  while (!glfwWindowShouldClose(window)) 
  {

        // OpenGL Draw commands
    draw();

        // Swap Frame Buffer in double buffering
    glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
    glfwPollEvents();

    reshapeWindow (window, width, height);

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) 
        { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
          last_update_time = current_time;
        }
      }
      // SOIL_free_image_data( ht_map );

      glfwTerminate();
      exit(EXIT_SUCCESS);
    }



    GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

  // Create the shaders
      GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
      GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
      std::string VertexShaderCode;
      std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
      if(VertexShaderStream.is_open())
      {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
          VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
      }

  // Read the Fragment Shader code from the file
      std::string FragmentShaderCode;
      std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
      if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
          FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
      }

      GLint Result = GL_FALSE;
      int InfoLogLength;

  // Compile Vertex Shader
      printf("Compiling shader : %s\n", vertex_file_path);
      char const * VertexSourcePointer = VertexShaderCode.c_str();
      glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
      glCompileShader(VertexShaderID);

  // Check Vertex Shader
      glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      std::vector<char> VertexShaderErrorMessage(InfoLogLength);
      glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
      fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

  // Compile Fragment Shader
      printf("Compiling shader : %s\n", fragment_file_path);
      char const * FragmentSourcePointer = FragmentShaderCode.c_str();
      glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
      glCompileShader(FragmentShaderID);

  // Check Fragment Shader
      glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
      glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
      fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

  // Link the program
      fprintf(stdout, "Linking program\n");
      GLuint ProgramID = glCreateProgram();
      glAttachShader(ProgramID, VertexShaderID);
      glAttachShader(ProgramID, FragmentShaderID);
      glLinkProgram(ProgramID);

  // Check the program
      glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
      glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
      glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
      fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

      glDeleteShader(VertexShaderID);
      glDeleteShader(FragmentShaderID);

      return ProgramID;
    }

  void quit(GLFWwindow *window)
    {
      glfwDestroyWindow(window);
      glfwTerminate();
      exit(EXIT_SUCCESS);
    }
  
  void draw3DObject (struct VAO* vao)
    {
    // Change the Fill Mode for this object
      glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
      glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
      glEnableVertexAttribArray(0);
    // Bind the VBO to use
      glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
      glEnableVertexAttribArray(1);
    // Bind the VBO to use
      glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
  }
