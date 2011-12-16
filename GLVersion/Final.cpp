////////////////////////////////////////////////////////////////////////
//
//   Harvard University
//   CS175 : Computer Graphics
//   Professor Steven Gortler
//
//	 Konlin Shen and Kane Hsieh
//   Final
//   12/4/11
//
////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#if __GNUG__
#   include <tr1/memory>
#endif

#include <GL/glew.h>
#ifdef __MAC__
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

#include "rigtform.h"
#include "cvec.h"
#include "matrix4.h"
#include "geometry.h"
#include "ppm.h"
#include "glsupport.h"
#include "arcball.h"

using namespace std;      // for string, vector, iostream, and other standard C++ stuff
using namespace std::tr1; // for shared_ptr

// G L O B A L S ///////////////////////////////////////////////////

// --------- IMPORTANT --------------------------------------------------------
// Before you start working on this assignment, set the following variable
// properly to indicate whether you want to use OpenGL 2.x with GLSL 1.0 or
// OpenGL 3.x+ with GLSL 1.3.
//
// Set g_Gl2Compatible = true to use GLSL 1.0 and g_Gl2Compatible = false to
// use GLSL 1.3. Make sure that your machine supports the version of GLSL you
// are using. In particular, on Mac OS X currently there is no way of using
// OpenGL 3.x with GLSL 1.3 when GLUT is used.
//
// If g_Gl2Compatible=true, shaders with -gl2 suffix will be loaded.
// If g_Gl2Compatible=false, shaders with -gl3 suffix will be loaded.
// To complete the assignment you only need to edit the shader files that get
// loaded
// ----------------------------------------------------------------------------
static const bool g_Gl2Compatible = false;


static const float g_frustFovY = 60.0;    // 60 degree field of view in y direction
static const float g_frustNear = -0.1;    // near plane
static const float g_frustFar = -50.0;    // far plane
static const float g_groundY = -2.0;      // y coordinate of the ground
static const float g_groundSize = 10.0;   // half the ground length

static int g_windowWidth = 512;
static int g_windowHeight = 512;
static bool g_mouseClickDown = false;    // is the mouse button pressed
static bool g_mouseLClickButton, g_mouseRClickButton, g_mouseMClickButton;
static int g_mouseClickX, g_mouseClickY; // coordinates for mouse click event
static int g_activeShader = 0;

static int currentObj = 2;
static int currentView = 0;
static int currentAuxFrame = 0;

struct ShaderState {
  GlProgram program;

  // Handles to uniform variables
  GLint h_uLight, h_uLight2;
  GLint h_uProjMatrix;
  GLint h_uModelViewMatrix;
  GLint h_uNormalMatrix;
  GLint h_uColor;

  // Handles to vertex attributes
  GLint h_aPosition;
  //GLint h_aPosition2;
  GLint h_aNormal;

  ShaderState(const char* vsfn, const char* fsfn) {
    readAndCompileShader(program, vsfn, fsfn);

    const GLuint h = program; // short hand

    // Retrieve handles to uniform variables
    h_uLight = safe_glGetUniformLocation(h, "uLight");
    h_uLight2 = safe_glGetUniformLocation(h, "uLight2");
    h_uProjMatrix = safe_glGetUniformLocation(h, "uProjMatrix");
    h_uModelViewMatrix = safe_glGetUniformLocation(h, "uModelViewMatrix");
    h_uNormalMatrix = safe_glGetUniformLocation(h, "uNormalMatrix");
    h_uColor = safe_glGetUniformLocation(h, "uColor");

    // Retrieve handles to vertex attributes
    h_aPosition = safe_glGetAttribLocation(h, "aPosition");
	//h_aPosition = safe_glGetAttribLocation(h, "aPosition2");
    h_aNormal = safe_glGetAttribLocation(h, "aNormal");

    if (!g_Gl2Compatible)
      glBindFragDataLocation(h, 0, "fragColor");
    checkGlErrors();
  }

};

static const int g_numShaders = 2;
static const char * const g_shaderFiles[g_numShaders][2] = {
  {"./shaders/basic-gl3.vshader", "./shaders/diffuse-gl3.fshader"},
  {"./shaders/basic-gl3.vshader", "./shaders/solid-gl3.fshader"}
};
static const char * const g_shaderFilesGl2[g_numShaders][2] = {
  {"./shaders/basic-gl2.vshader", "./shaders/diffuse-gl2.fshader"},
  {"./shaders/basic-gl2.vshader", "./shaders/solid-gl2.fshader"}
};
static vector<shared_ptr<ShaderState> > g_shaderStates; // our global shader states

// --------- Geometry

// Macro used to obtain relative offset of a field within a struct
#define FIELD_OFFSET(StructType, field) &(((StructType *)0)->field)

// A vertex with floating point position and normal
struct VertexPN {
  Cvec3f p, n;

  VertexPN() {}
  VertexPN(float x, float y, float z,
           float nx, float ny, float nz)
    : p(x,y,z), n(nx, ny, nz)
  {}

  // Define copy constructor and assignment operator from GenericVertex so we can
  // use make* functions from geometry.h
  VertexPN(const GenericVertex& v) {
    *this = v;
  }

  VertexPN& operator = (const GenericVertex& v) {
    p = v.pos;
    n = v.normal;
    return *this;
  }
};

struct Geometry {
  GlBufferObject vbo, ibo;
  int vboLen, iboLen;

  Geometry(VertexPN *vtx, unsigned short *idx, int vboLen, int iboLen) {
    this->vboLen = vboLen;
    this->iboLen = iboLen;

    // Now create the VBO and IBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vboLen, vtx, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * iboLen, idx, GL_STATIC_DRAW);
  }

  void draw(const ShaderState& curSS) {
    // Enable the attributes used by our shader
    safe_glEnableVertexAttribArray(curSS.h_aPosition);
    safe_glEnableVertexAttribArray(curSS.h_aNormal);
	
    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    safe_glVertexAttribPointer(curSS.h_aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), FIELD_OFFSET(VertexPN, p));
    safe_glVertexAttribPointer(curSS.h_aNormal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), FIELD_OFFSET(VertexPN, n));

    // bind ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // draw!
    glDrawElements(GL_TRIANGLES, iboLen, GL_UNSIGNED_SHORT, 0);

    // Disable the attributes used by our shader
    safe_glDisableVertexAttribArray(curSS.h_aPosition);
    safe_glDisableVertexAttribArray(curSS.h_aNormal);
  }
};


// Vertex buffer and index buffer associated with the ground and cube geometry
static shared_ptr<Geometry> ground, cube1, cube2, sphere, skySphere;

// --------- Scene

static const Cvec3 g_light1(2.0, 3.0, 14.0), g_light2(-2, -3.0, -5.0);  // define two lights positions in world space
static RigTForm g_skyRbt = RigTForm(Cvec3(0.0, 0.25, 4)); 
static RigTForm g_objectRbt[4] = {RigTForm(Cvec3(-1,0,0)), RigTForm(Cvec3(1,0,0)), RigTForm(Cvec3(0,0,0)), RigTForm(Cvec3(1, 1, 1))};  // define 3 objects
static Cvec3f g_originalObjectColors[3] = {Cvec3f(1, 0, 0), Cvec3f(0,0,1), Cvec3f(0,1,0)};
static Cvec3f g_objectColors[4] = {Cvec3f(1, 0, 0), Cvec3f(0,0,1), Cvec3f(0,1,0), Cvec3f(1, 1, 0)};
RigTForm eyeRbt;

//CAMERA PARAMETERS
static float g_focalLength = .05;
static float g_aperture = 1.4;
static float g_circleOfConfusion = .0019;
static int g_ApertureCounter = 0;
static float pixelFactor = 100;

///////////////// END OF G L O B A L S //////////////////////////////////////////////////


static void initGround() {
  // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
  VertexPN vtx[4] = {
    VertexPN(-g_groundSize, g_groundY, -g_groundSize, 0, 1, 0),
    VertexPN(-g_groundSize, g_groundY,  g_groundSize, 0, 1, 0),
    VertexPN( g_groundSize, g_groundY,  g_groundSize, 0, 1, 0),
    VertexPN( g_groundSize, g_groundY, -g_groundSize, 0, 1, 0),
  };
  unsigned short idx[] = {0, 1, 2, 0, 2, 3};
  ground.reset(new Geometry(&vtx[0], &idx[0], 4, 6));
}

static void initCubes() {
  int ibLen, vbLen;
  getCubeVbIbLen(vbLen, ibLen);

  // Temporary storage for cube geometry
  vector<VertexPN> vtx(vbLen);
  vector<unsigned short> idx(ibLen);

  makeCube(1, vtx.begin(), idx.begin());
  cube1.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));

  makeCube(1, vtx.begin(), idx.begin());
  cube2.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initSphere(){
	int ibLen, vbLen;
	getSphereVbIbLen(30,30,vbLen,ibLen);

	vector<VertexPN> vtx(vbLen);
	vector<unsigned short> idx(ibLen);

	makeSphere(1,30,30,vtx.begin(),idx.begin());
	sphere.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));

	makeSphere(.05, 30, 30, vtx.begin(), idx.begin());
	skySphere.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));
}

// takes a projection matrix and send to the the shaders
static void sendProjectionMatrix(const ShaderState& curSS, const Matrix4& projMatrix) {
  GLfloat glmatrix[16];
  projMatrix.writeToColumnMajorMatrix(glmatrix); // send projection matrix
  safe_glUniformMatrix4fv(curSS.h_uProjMatrix, glmatrix);
}

// takes MVM and its normal matrix to the shaders
static void sendModelViewNormalMatrix(const ShaderState& curSS, const Matrix4& MVM, const Matrix4& NMVM) {
  GLfloat glmatrix[16];
  MVM.writeToColumnMajorMatrix(glmatrix); // send MVM
  safe_glUniformMatrix4fv(curSS.h_uModelViewMatrix, glmatrix);

  NMVM.writeToColumnMajorMatrix(glmatrix); // send NMVM
  safe_glUniformMatrix4fv(curSS.h_uNormalMatrix, glmatrix);
}

static void drawStuff() {
  // short hand for current shader state
  const ShaderState& curSS = *g_shaderStates[g_activeShader];

  // build & send proj. matrix to vshader
  const Matrix4 projmat = Matrix4::makeProjection(
    g_frustFovY, g_windowWidth / static_cast <double> (g_windowHeight),
    g_frustNear, g_frustFar);
  sendProjectionMatrix(curSS, projmat);

  RigTForm eyeRbt;

  // define eye
  if(currentView == 0)
	  eyeRbt = g_skyRbt;
  else if (currentView == 1)
	  eyeRbt = g_objectRbt[0];
  else
	  eyeRbt = g_objectRbt[1];

  const RigTForm invEyeRbt = inv(eyeRbt);

  const Cvec3 eyeLight1 = Cvec3(invEyeRbt * Cvec4(g_light1, 1)); // g_light1 position in eye coordinates
  const Cvec3 eyeLight2 = Cvec3(invEyeRbt * Cvec4(g_light2, 1)); // g_light2 position in eye coordinates
  safe_glUniform3f(curSS.h_uLight, eyeLight1[0], eyeLight1[1], eyeLight1[2]);
  safe_glUniform3f(curSS.h_uLight2, eyeLight2[0], eyeLight2[1], eyeLight2[2]);

  // draw ground
  // ===========
  const Matrix4 groundRbt = Matrix4();  // identity
  Matrix4 MVM = rigTFormToMatrix(invEyeRbt) * groundRbt;
  Matrix4 NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, 0.1, 0.95, 0.1); // set color
  ground->draw(curSS);

  // draw cubes
  // ==========
  MVM = rigTFormToMatrix(invEyeRbt * g_objectRbt[0]);
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, g_objectColors[0][0], g_objectColors[0][1], g_objectColors[0][2]);
  cube1->draw(curSS);

  MVM = rigTFormToMatrix(invEyeRbt * g_objectRbt[1]);
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, g_objectColors[1][0], g_objectColors[1][1], g_objectColors[1][2]);
  cube2->draw(curSS);


  //draw sphere
  //===========
  if(currentView == 0){
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw wireframe
  if(currentObj != 2)
	g_objectRbt[2]=g_objectRbt[currentObj];
  else
	  g_objectRbt[2].setTranslation(Cvec3(0,0,0));
  MVM = rigTFormToMatrix(invEyeRbt * g_objectRbt[2]);
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, g_objectColors[2][0], g_objectColors[2][1], g_objectColors[2][2]);
  sphere->draw(curSS);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

    //draw center of sky-axis
  //===========
  MVM = rigTFormToMatrix(invEyeRbt * g_objectRbt[3]);
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, g_objectColors[3][0], g_objectColors[3][1], g_objectColors[3][2]);
  skySphere->draw(curSS);


}

static void calculateDOF(int index){
	float focalLength = g_focalLength * pixelFactor;
	float circleOfConfusion = g_circleOfConfusion * pixelFactor;
	Cvec3 c = g_skyRbt.getTranslation();
	
	float focalDistance = sqrt(pow(c(0) - g_objectRbt[3].getTranslation()(0),2) + pow(c(1)- g_objectRbt[3].getTranslation()(1),2) + pow(c(2)- g_objectRbt[3].getTranslation()(2),2));
	float hyperfocalDistance = pow(focalLength,2)/(circleOfConfusion * g_aperture/focalLength * 10) + focalLength;
	float nearPlane = focalDistance * (hyperfocalDistance - focalLength)/(hyperfocalDistance + focalDistance - 2 * focalLength);
	float farPlane = focalDistance * (hyperfocalDistance - focalLength)/(hyperfocalDistance - focalDistance);
	

	cout << "Cam Coordinates: " << c(0) << " " << c(1) << " " << c(2) << endl;
	cout << "ObjectWRTSky coordinates: " << g_objectRbt[index].getTranslation()(0) << " " << g_objectRbt[index].getTranslation()(1) << " " << g_objectRbt[index].getTranslation()(2) << endl;
	cout << "eyeDepth: " << c(2) << endl;
	cout << "hyperfocal distance: " << hyperfocalDistance << endl;
	cout << "nearPlane: " << nearPlane << endl;
	cout << "farPlane: " << farPlane << endl;
	cout << "Object Depth: " << g_objectRbt[index].getTranslation()(2) << endl;

	if(g_objectRbt[index].getTranslation()(2) < farPlane || g_objectRbt[index].getTranslation()(2) > nearPlane)
		g_objectColors[index] = Cvec3f(1,1,1);
	else{
		cout << "back to original color" << endl;
		g_objectColors[index] = g_originalObjectColors[index];
	}
}

static void display() {
  glUseProgram(g_shaderStates[g_activeShader]->program);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                   // clear framebuffer color&depth

  drawStuff();

  glutSwapBuffers();                                    // show the back buffer (where we rendered stuff)

  checkGlErrors();
}

static void reshape(const int w, const int h) {
  g_windowWidth = w;
  g_windowHeight = h;
  glViewport(0, 0, w, h);
  std::cerr << "Size of window is now " << w << "x" << h << std::endl;
  glutPostRedisplay();
}

static void motion(const int x, const int y) {
  const double dx = x - g_mouseClickX;
  const double dy = g_windowHeight - y - 1 - g_mouseClickY;
  
  //initialize the outCenter and outRadius for the screenSpaceCircle 
  Cvec2 outCenter;
  double outRadius;
  
  //initialize the projection matrix for the screenSpaceCircle
  const Matrix4 projmat = Matrix4::makeProjection(
  g_frustFovY, g_windowWidth / static_cast <double> (g_windowHeight),
  g_frustNear, g_frustFar);
 
  if(currentView == 0)
	  eyeRbt = g_skyRbt;
  else if (currentView == 1)
	  eyeRbt = g_objectRbt[0];
  else
	  eyeRbt = g_objectRbt[1];
  
  //gets the center for the screenSpaceCircle by passing in the center of the sphere in eye-coordinates
  Cvec3 center = (inv(eyeRbt) * g_objectRbt[2]).getTranslation();

  //getsthe screenSpaceCircle
  getScreenSpaceCircle(center, 1, projmat, g_frustNear, g_frustFovY, g_windowWidth, g_windowHeight, outCenter, outRadius);
  
  //get the two screen space vectors
  Cvec2 p1((g_mouseClickX+dx)-outCenter(0), (dy + g_mouseClickY)-outCenter(1));
  Cvec2 p2(g_mouseClickX-outCenter(0), g_mouseClickY-outCenter(1));

  //clamp if we go outside the radius of the sphere
  double dist1 = sqrt(pow(p1(0),2) + pow(p1(1),2));
  if(dist1 > outRadius){
	  p1 = p1 * outRadius/(dist1+10);  //+10 to avoid random rounding errors and stuff
  }

  double dist2 = sqrt(pow(p2(0),2) + pow(p2(1),2));
  if (dist2 > outRadius){
	  p2 = p2 * outRadius/(dist2+10);
  }
  
  //Z-components for the projection
  double currentZ = sqrt(pow(outRadius,2) - pow(p1(0),2) - pow(p1(1),2));
  double transZ = sqrt(pow(outRadius,2) - pow(p2(0),2) - pow(p2(1),2));

  //create two vectors for each mouse click with the tails at the origin of the sphere
  Cvec3 currentV(p1, currentZ);
  Cvec3 transV(p2, transZ);

  //create two quaternions with normalized vectors
  Quat qV1(0, normalize(currentV));
  Quat qV2(0, normalize(transV));

  //calculate the rotation quaternion
  Quat Q = qV2 * inv(qV1);

  RigTForm m;
  if (g_mouseLClickButton && !g_mouseRClickButton){ // left button down? 
	  m.setRotation(Q);  //set the rotation quaternion
  }
  else if (g_mouseRClickButton && !g_mouseLClickButton) { // right button down?
	if(currentObj==2 && currentAuxFrame==0)
		m.setTranslation(Cvec3(dx, dy, 0) * -0.01);
	else
		m.setTranslation(Cvec3(dx, dy, 0) * 0.01);
  }	
  else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {  // middle or (left and right) button down?
    m.setTranslation(Cvec3(0, 0, -dy) * 0.01);
  }

  RigTForm auxFrame;  //initialize the auxillary frame

  if (g_mouseClickDown) {
	  if(currentObj != 2){
		  m.setRotation(inv(m.getRotation()));
		  auxFrame.setTranslation(g_objectRbt[currentObj].getTranslation());
		  auxFrame.setRotation(eyeRbt.getRotation());
		  g_objectRbt[currentObj] = auxFrame * m * inv(auxFrame) * g_objectRbt[currentObj];//rotate around the object frame, translate around the sky frame
	  }
	  else if (currentObj == 2 && currentAuxFrame == 0){
		  auxFrame.setRotation(eyeRbt.getRotation());         
		  g_skyRbt = auxFrame * m * inv(auxFrame) * g_skyRbt;  //world-sky aux frame
	  }
	  else if (currentObj == 2 && currentAuxFrame == 1){
		  auxFrame.setTranslation(eyeRbt.getTranslation());
		  auxFrame.setRotation(eyeRbt.getRotation());
		  g_skyRbt = auxFrame * m * inv(auxFrame) * g_skyRbt;  //sky-sky aux frame
	  }
    glutPostRedisplay(); // we always redraw if we change the scene
  }

  g_mouseClickX = x;
  g_mouseClickY = g_windowHeight - y - 1;
}

static void mouse(const int button, const int state, const int x, const int y) {
  g_mouseClickX = x;
  g_mouseClickY = g_windowHeight - y - 1;  // conversion from GLUT window-coordinate-system to OpenGL window-coordinate-system

  g_mouseLClickButton |= (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
  g_mouseRClickButton |= (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN);
  g_mouseMClickButton |= (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN);

  g_mouseLClickButton &= !(button == GLUT_LEFT_BUTTON && state == GLUT_UP);
  g_mouseRClickButton &= !(button == GLUT_RIGHT_BUTTON && state == GLUT_UP);
  g_mouseMClickButton &= !(button == GLUT_MIDDLE_BUTTON && state == GLUT_UP);

  g_mouseClickDown = g_mouseLClickButton || g_mouseRClickButton || g_mouseMClickButton;
}

static void keyboard(const unsigned char key, const int x, const int y) {
  switch (key) {
  case 27:
    exit(0);                                  // ESC
  case 'h':
    cout << " ============== H E L P ==============\n\n"
    << "h\t\thelp menu\n"
    << "s\t\tsave screenshot\n"
    << "f\t\tToggle flat shading on/off.\n"
    << "o\t\tCycle object to edit\n"
    << "v\t\tCycle view\n"
    << "drag left mouse to rotate\n" << endl;
    break;
  case 's':
    glFlush();
    writePpmScreenshot(g_windowWidth, g_windowHeight, "out.ppm");
    break;
  case 'f':
    g_activeShader ^= 1;
    break;
  case 'm':  //toggles the auxillary frame for the sky eye
	if(currentAuxFrame == 0){
		currentAuxFrame = 1;
		cout << "Manipulating about the Sky-Sky frame\n";
	}
	else{
		currentAuxFrame = 0;
		cout << "Manipulating about the World-Sky frame\n";
	}
    break;
  case 'o':  //cycles through the various objects
	if(currentObj == 0){
		currentObj = 1;
		cout << "Current Object is Blue Cube\n";
	}
	else if (currentObj == 1){
		currentObj = 2;
		cout << "Current Object is Sky\n";
	}
	else if (currentObj == 2){
		currentObj = 3;
		cout << "Current Obj is Focus\n";
	}
	else{
		currentObj = 0;
		cout << "Current Object is Red Cube\n";
	}
	break;
  case 'v': //cycles through the various views
	 if(currentView == 0){
		currentView = 1;
		cout << "Current Eye is Red Cube\n";
	 }
	 else if (currentView == 1){
		currentView = 2;
		cout << "Current Eye is Blue Cube\n";
	 }
	 else if (currentView == 2){
		currentView = 0;
		cout << "Current Eye is Sky\n";
	 }
	 break;
	case 'a':
	  g_ApertureCounter++;
	  switch(g_ApertureCounter%10){
		case 0:
			g_aperture = 1.4;
			break;
		case 1:
			g_aperture = 2;
			break;
		case 2:
			g_aperture = 2.8;
			break;
		case 3:
			g_aperture = 4;
			break;
		case 4:
			g_aperture = 5.6;
			break;
		case 5:
			g_aperture = 8;
			break;
		case 6:
			g_aperture = 11;
			break;
		case 7:
			g_aperture = 16;
			break;
		case 8:
			g_aperture = 22;
			break;
		case 9:
			g_aperture = 32;
			break;
	  }
	  cout << "F-number is: f/" << g_aperture << endl;
	  break;
	case ' ':
		for(int i = 0; i < 2; i++)
			calculateDOF(i);

		glFlush();
		writePpmScreenshot(g_windowWidth, g_windowHeight, "snapshot.ppm");
		break;
	case '+':
		pixelFactor *= 10;
		cout << "Setting Pixel Map to: " << pixelFactor << endl;
		break;
	case '-':
		pixelFactor /= 10;
		cout << "Setting Pixel Map to: " << pixelFactor << endl;
		break;
  }
  glutPostRedisplay();
}

static void initGlutState(int argc, char * argv[]) {
  glutInit(&argc, argv);                                  // initialize Glut based on cmd-line args
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);  //  RGBA pixel channels and double buffering
  glutInitWindowSize(g_windowWidth, g_windowHeight);      // create a window
  glutCreateWindow("Final Project");                       // title the window

  glutDisplayFunc(display);                               // display rendering callback
  glutReshapeFunc(reshape);                               // window reshape callback
  glutMotionFunc(motion);                                 // mouse movement callback
  glutMouseFunc(mouse);                                   // mouse click callback
  glutKeyboardFunc(keyboard);
}

static void initGLState() {
  glClearColor(128./255., 200./255., 255./255., 0.);
  glClearDepth(0.);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);
  glReadBuffer(GL_BACK);
  if (!g_Gl2Compatible)
    glEnable(GL_FRAMEBUFFER_SRGB);
}

static void initShaders() {
  g_shaderStates.resize(g_numShaders);
  for (int i = 0; i < g_numShaders; ++i) {
    if (g_Gl2Compatible)
      g_shaderStates[i].reset(new ShaderState(g_shaderFilesGl2[i][0], g_shaderFilesGl2[i][1]));
    else
      g_shaderStates[i].reset(new ShaderState(g_shaderFiles[i][0], g_shaderFiles[i][1]));
  }
}

static void initGeometry() {
  initGround();
  initCubes();
  initSphere();
}

int main(int argc, char * argv[]) {
  try {
    initGlutState(argc,argv);

    glewInit(); // load the OpenGL extensions

    cout << (g_Gl2Compatible ? "Will use OpenGL 2.x / GLSL 1.0" : "Will use OpenGL 3.x / GLSL 1.3") << endl;
    if ((!g_Gl2Compatible) && !GLEW_VERSION_3_0)
      throw runtime_error("Error: card/driver does not support OpenGL Shading Language v1.3");
    else if (g_Gl2Compatible && !GLEW_VERSION_2_0)
      throw runtime_error("Error: card/driver does not support OpenGL Shading Language v1.0");

    initGLState();
    initShaders();
    initGeometry();

    glutMainLoop();
    return 0;
  }
  catch (const runtime_error& e) {
    cout << "Exception caught: " << e.what() << endl;
    return -1;
  }
}