////////////////////////////////////////////////////////////////////////
//
//   Harvard University
//   CS175 : Computer Graphics
//   Professor Steven Gortler
//
//	 Kane Hsieh, Konlin Shen
//   Final Project
//   12/9/11
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
#include "asstcommon.h"
#include "scenegraph.h"
#include "drawer.h"
#include "picker.h"

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

static const int PICKING_SHADER = 2; // index of the picking shader is g_shaerFiles
static const int g_numShaders = 3; // 3 shaders instead of 2
static const char * const g_shaderFiles[g_numShaders][2] = {
  {"./shaders/basic-gl3.vshader", "./shaders/diffuse-gl3.fshader"},
  {"./shaders/basic-gl3.vshader", "./shaders/solid-gl3.fshader"},
  {"./shaders/basic-gl3.vshader", "./shaders/pick-gl3.fshader"}
};
static const char * const g_shaderFilesGl2[g_numShaders][2] = {
  {"./shaders/basic-gl2.vshader", "./shaders/diffuse-gl2.fshader"},
  {"./shaders/basic-gl2.vshader", "./shaders/solid-gl2.fshader"},
  {"./shaders/basic-gl2.vshader", "./shaders/pick-gl2.fshader"}
};
static vector<shared_ptr<ShaderState> > g_shaderStates; // our global shader states


// Vertex buffer and index buffer associated with the ground and cube geometry
static shared_ptr<Geometry> ground, cube1, sphere; //cube2

// --------- Scene

static const Cvec3 g_light1(2.0, 3.0, 14.0), g_light2(-2, -3.0, -5.0);  // define two lights positions in world space
static RigTForm g_skyRbt = RigTForm(Cvec3(0.0, 0.25, 4)); 
static RigTForm g_objectRbt[3] = {RigTForm(Cvec3(-1,0,0)), RigTForm(Cvec3(1,0,0)), RigTForm(Cvec3(0,0,0))};  // define 3 objects
static Cvec3f g_objectColors[3] = {Cvec3f(1, 0, 0), Cvec3f(0,0,1), Cvec3f(0,1,0)};


static shared_ptr<SgRootNode> g_world;
static shared_ptr<SgRbtNode> g_skyNode, g_groundNode, g_robot1Node, g_robot2Node;
static shared_ptr<SgRbtNode> g_currentPickedRbtNode, g_viewNode;


//picking state
bool g_PickState = false;

//CAMERA PARAMETERS
static float g_focalLength = .05;
static float g_aperture = 1.4;
static float g_circleOfConfusion = 0.019;
static int g_ApertureCounter = 0;

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
}

static void initSphere(){
	int ibLen, vbLen;
	getSphereVbIbLen(30,30,vbLen,ibLen);

	vector<VertexPN> vtx(vbLen);
	vector<unsigned short> idx(ibLen);

	makeSphere(1,30,30,vtx.begin(),idx.begin());
	sphere.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));
}

static void drawStuff(const ShaderState& curSS, bool picking) {
  // build & send proj. matrix to vshader
  const Matrix4 projmat = Matrix4::makeProjection(
    g_frustFovY, g_windowWidth / static_cast <double> (g_windowHeight),
    g_frustNear, g_frustFar);
  
  sendProjectionMatrix(curSS, projmat);

  // define eye
  if(currentView == 0)
	  g_viewNode = g_skyNode;
  else if (currentView == 1)
	  g_viewNode = g_robot1Node;
  else
	  g_viewNode = g_robot2Node;

  RigTForm eyeRbt = getPathAccumRbt(g_world,g_viewNode);

  const RigTForm invEyeRbt = inv(eyeRbt);
  const Cvec3 eyeLight1 = Cvec3(invEyeRbt * Cvec4(g_light1, 1)); // g_light1 position in eye coordinates
  const Cvec3 eyeLight2 = Cvec3(invEyeRbt * Cvec4(g_light2, 1)); // g_light2 position in eye coordinates
  safe_glUniform3f(curSS.h_uLight, eyeLight1[0], eyeLight1[1], eyeLight1[2]);
  safe_glUniform3f(curSS.h_uLight2, eyeLight2[0], eyeLight2[1], eyeLight2[2]);

  if (!picking) {
	Drawer drawer(invEyeRbt, curSS);
	g_world->accept(drawer);
  }
  else {
	Picker picker(invEyeRbt, curSS);
    g_world->accept(picker);
    glFlush();
    g_currentPickedRbtNode = picker.getRbtNodeAtXY(g_mouseClickX, g_mouseClickY);
	if(g_currentPickedRbtNode == NULL)
		g_currentPickedRbtNode = g_skyNode;
  }
  //draw sphere
  //===========
  if(currentView == 0){
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // draw wireframe
  if(g_currentPickedRbtNode != g_skyNode)
	  g_objectRbt[2]=getPathAccumRbt(g_world, g_currentPickedRbtNode);
  else
	  g_objectRbt[2].setTranslation(Cvec3(0,0,0));
  Matrix4 MVM = rigTFormToMatrix(invEyeRbt * g_objectRbt[2]);
  Matrix4 NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, g_objectColors[2][0], g_objectColors[2][1], g_objectColors[2][2]);
  sphere->draw(curSS);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}

static void pick() {
  // We need to set the clear color to black, for pick rendering.
  // so let's save the clear color
  GLdouble clearColor[4];
  glGetDoublev(GL_COLOR_CLEAR_VALUE, clearColor);

  glClearColor(0, 0, 0, 0);

  // using PICKING_SHADER as the shader
  glUseProgram(g_shaderStates[PICKING_SHADER]->program);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawStuff(*g_shaderStates[PICKING_SHADER], true);

  // Uncomment below and comment out the glutPostRedisplay in mouse(...) call back
  // to see result of the pick rendering pass
   //glutSwapBuffers();

  //Now set back the clear color
  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

  checkGlErrors();
}

static void display() {
  glUseProgram(g_shaderStates[g_activeShader]->program);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  drawStuff(*g_shaderStates[g_activeShader], false);

  glutSwapBuffers();

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
  
  //define eye
  RigTForm eyeRbt = getPathAccumRbt(g_world, g_viewNode);
  
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
  Quat Q;

  RigTForm m; 

  if (g_mouseLClickButton && !g_mouseRClickButton){ // left button down? 
	  if((g_viewNode == g_skyNode && currentAuxFrame == 1) || (g_viewNode != g_skyNode && g_currentPickedRbtNode == g_viewNode)){ //ego motion case
		  Q = Quat::makeXRotation(-dy) * Quat::makeYRotation(dx);
	  }
	  else
		  Q = qV2 * inv(qV1);

	  m.setRotation(Q);  //set the rotation quaternion
  }
  else if (g_mouseRClickButton && !g_mouseLClickButton) { // right button down?
		if(g_currentPickedRbtNode!=g_skyNode)
			m.setTranslation(Cvec3(dx, dy, 0) * 0.01);
		else
			m.setTranslation(Cvec3(dx, dy, 0) * -0.01);
  }	
  else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {  // middle or (left and right) button down?
    m.setTranslation(Cvec3(0, 0, -dy) * 0.01);
  }

  RigTForm auxFrame; //initialize auxiliary frame

  if (g_mouseClickDown) {	
	  if (g_currentPickedRbtNode == g_skyNode && currentAuxFrame == 0){
		auxFrame.setRotation(eyeRbt.getRotation());
		g_currentPickedRbtNode->setRbt(auxFrame * m * inv(auxFrame) * g_currentPickedRbtNode->getRbt());  //world-sky aux frame
	  }
	  else if (g_currentPickedRbtNode == g_skyNode && currentAuxFrame == 1 || g_viewNode != g_skyNode && g_currentPickedRbtNode == g_viewNode){
		m = inv(m);
		auxFrame.setTranslation(eyeRbt.getTranslation());
	    auxFrame.setRotation(eyeRbt.getRotation());
		g_currentPickedRbtNode->setRbt(auxFrame * m * inv(auxFrame) * g_currentPickedRbtNode->getRbt()); //ego motion
	  }
	  else if (g_currentPickedRbtNode != g_groundNode && g_viewNode != g_currentPickedRbtNode){
		m.setRotation(inv(m.getRotation()));
		RigTForm A_t = getPathAccumRbt(g_world, g_currentPickedRbtNode);
		RigTForm A_r = getPathAccumRbt(g_world, g_viewNode);
		auxFrame.setTranslation(A_t.getTranslation());
		auxFrame.setRotation(A_r.getRotation());  
		RigTForm oneBeforeFrame = getPathAccumRbt(g_world, g_currentPickedRbtNode,1);
		RigTForm A_s = inv(oneBeforeFrame) * auxFrame;
		g_currentPickedRbtNode->setRbt(A_s * m * inv(A_s) * g_currentPickedRbtNode->getRbt());
	  }
	  glutPostRedisplay(); // we always redraw if we change the scene
  }

  g_mouseClickX = x;
  g_mouseClickY = g_windowHeight - y - 1;
}

static void calculateDOF(RigTForm rbt){
	Cvec3 c = rbt.getTranslation();
	float hyperfocalDistance = pow(g_focalLength,2)/(g_circleOfConfusion * g_aperture/g_focalLength) + g_focalLength;
	float nearPlane = c(3) * (hyperfocalDistance - g_focalLength)/(hyperfocalDistance + c(3) - 2 * g_focalLength);
	float farPlane = c(3) * (hyperfocalDistance - g_focalLength)/(hyperfocalDistance - c(3));

	if(c(3) > farPlane || c(3) < nearPlane){

	}

	glFlush();
    writePpmScreenshot(g_windowWidth, g_windowHeight, "snapshot.ppm");

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

   if(g_PickState){
	  pick();
	  g_PickState = false;
   }

   glutPostRedisplay();
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
  case 'p':
	 g_PickState = !g_PickState;
	 cout << "Picking Mode On" << endl;
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

static void constructRobot(shared_ptr<SgTransformNode> base, const Cvec3& color) {

  const double ARM_LEN = 0.7,
            ARM_THICK = 0.25,
            TORSO_LEN = 1.5,
            TORSO_THICK = 0.25,
            TORSO_WIDTH = 1,
			HEAD_RADIUS = .4;


  const int NUM_JOINTS = 10,
            NUM_SHAPES = 10;

  struct JointDesc {
    int parent;
    float x, y, z;
  };

  JointDesc jointDesc[NUM_JOINTS] = {
    {-1}, // torso
    {0,  TORSO_WIDTH/2, TORSO_LEN/2, 0}, // upper right arm
    {1,  ARM_LEN, 0, 0},// lower right arm
	{0,  -TORSO_WIDTH/2, TORSO_LEN/2, 0},
	{3, -ARM_LEN, 0, 0},
	{0, 0, TORSO_LEN/2, 0},
	{0, TORSO_WIDTH/2-ARM_THICK/2, -TORSO_LEN/2, 0},
	{6, 0, -ARM_LEN, 0},
	{0, -(TORSO_WIDTH/2-ARM_THICK/2), -TORSO_LEN/2, 0},
	{8, 0, -ARM_LEN, 0},

  };

  struct ShapeDesc {
    int parentJointId;
    float x, y, z, sx, sy, sz;
    shared_ptr<Geometry> geometry;
  };

  ShapeDesc shapeDesc[NUM_SHAPES] = {
    {0, 0,         0, 0, TORSO_WIDTH, TORSO_LEN, TORSO_THICK, cube1}, // torso
    {1, ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, cube1}, // upper right arm
    {2, ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK * 2/3, ARM_THICK, cube1},// lower right arm
	{3, -ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, cube1},
	{4, -ARM_LEN/2, 0, 0, ARM_LEN, ARM_THICK * 2/3, ARM_THICK, cube1},
	{5, 0, HEAD_RADIUS, 0, HEAD_RADIUS, HEAD_RADIUS, HEAD_RADIUS, sphere},
	{6, 0, -ARM_LEN/2, 0, ARM_THICK, ARM_LEN, ARM_THICK, cube1},
	{7, 0, -ARM_LEN/2, 0, ARM_THICK*2/3, ARM_LEN, ARM_THICK, cube1},
	{8, 0, -ARM_LEN/2, 0, ARM_THICK, ARM_LEN, ARM_THICK, cube1},
	{9, 0, -ARM_LEN/2, 0, ARM_THICK*2/3, ARM_LEN, ARM_THICK, cube1},
  };

  shared_ptr<SgTransformNode> jointNodes[NUM_JOINTS];

  for (int i = 0; i < NUM_JOINTS; ++i) {
    if (jointDesc[i].parent == -1)
      jointNodes[i] = base;
    else {
      jointNodes[i].reset(new SgRbtNode(RigTForm(Cvec3(jointDesc[i].x, jointDesc[i].y, jointDesc[i].z))));
      jointNodes[jointDesc[i].parent]->addChild(jointNodes[i]);
    }
  }
  for (int i = 0; i < NUM_SHAPES; ++i) {
    shared_ptr<SgGeometryShapeNode> shape(
      new SgGeometryShapeNode(shapeDesc[i].geometry, 
      color, 
      Cvec3(shapeDesc[i].x, shapeDesc[i].y, shapeDesc[i].z),
      Cvec3(0, 0, 0),
      Cvec3(shapeDesc[i].sx, shapeDesc[i].sy, shapeDesc[i].sz)));
    jointNodes[shapeDesc[i].parentJointId]->addChild(shape);
  }
}

static void initScene() {
  g_world.reset(new SgRootNode());

  g_skyNode.reset(new SgRbtNode(RigTForm(Cvec3(0.0, 0.25, 4.0))));

  g_currentPickedRbtNode = g_skyNode;

  g_viewNode = g_skyNode;
  
  g_groundNode.reset(new SgRbtNode());
  g_groundNode->addChild(shared_ptr<SgGeometryShapeNode>(
                           new SgGeometryShapeNode(ground, Cvec3(0.1, 0.95, 0.1))));

  //g_robot1Node.reset(new SgRbtNode(RigTForm(Cvec3(-2, 1, 0))));
  //g_robot2Node.reset(new SgRbtNode(RigTForm(Cvec3(2, 1, 0))));

  //constructRobot(g_robot1Node, Cvec3(1, 0, 0)); // a Red robot
  //constructRobot(g_robot2Node, Cvec3(0, 0, 1)); // a Blue robot

  g_world->addChild(g_skyNode);
  g_world->addChild(g_groundNode);
  g_world->addChild(g_robot1Node);
  g_world->addChild(g_robot2Node);
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
	initScene();

    glutMainLoop();
    return 0;
  }
  catch (const runtime_error& e) {
    cout << "Exception caught: " << e.what() << endl;
    return -1;
  }
} 