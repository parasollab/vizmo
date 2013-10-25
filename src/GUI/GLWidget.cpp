#include "GLWidget.h"

#include <numeric>
#include <ctime>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QGLWidget>

#include <glut.h>

#include "MainWindow.h"
#include "Models/Vizmo.h"
#include "Utilities/GL/Camera.h"
#include "Utilities/GL/Font.h"
#include "Utilities/GL/PickBox.h"
#include "Utilities/GL/TransformTool.h"

bool SHIFT_CLICK = false;

///////////////////////////////////////////////////////////////////////////////
//This class handle opengl features

GLWidget::GLWidget(QWidget* _parent, MainWindow* _mainWindow)
  : QGLWidget(_parent){
    m_mainWindow = _mainWindow;
    setMinimumSize(400, 505); //original size: 400 x 600
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::StrongFocus);

    m_takingSnapShot=false;
    m_showAxis = true;
    m_showFrameRate = false;
    m_cdOn = false;
}

void
GLWidget::ToggleSelectionSlot(){
  m_takingSnapShot = !m_takingSnapShot;
}

void
GLWidget::ResetCamera(){
  GetCameraFactory().GetCurrentCamera()->Set(Point3d(0, 0, 4*GetVizmo().GetEnvRadius()), 0, 0);
}

//used as callback for gli
inline vector<Model*>&
VizmoSelect(const Box& _box){
  GetVizmo().Select(_box);
  return GetVizmo().GetSelectedModels();
}

void
GLWidget::initializeGL(){

  /*Setup light and material properties*/
  SetLight();

  /*others*/
  glEnable(GL_DEPTH_TEST);

  glClearColor(1, 1, 1, 0);
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  glLineStipple(2, 0xAAAA);

  //create models
  SetPickingFunction(VizmoSelect);
}

void
GLWidget::resizeGL(int _w, int _h) {
  GetTransformTool().SetWindowSize(_w, _h);
  GetPickBox().SetWinSize(_w, _h);

  glViewport(0, 0, _w, _h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, ((GLfloat)_w)/((GLfloat)_h), 1, 1500);
}

void
GLWidget::paintGL(){

  //start clock
  clock_t startTime = clock();

  //Init Draw
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_mainWindow->InitVizmo();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //draw camera
  GetCameraFactory().GetCurrentCamera()->Draw();

  //draw pick box
  GetPickBox().Draw();

  //draw transform tool
  GetTransformTool().Draw();

  //draw axis
  DrawAxis();

  //set lights
  SetLightPos();

  //draw scene
  GetVizmo().Display();

  //stop clock, update frametimes, and compute framerate
  clock_t endTime = clock();

  double time = double(endTime - startTime)/CLOCKS_PER_SEC;
  m_frameTimes.push_back(time);
  if(m_frameTimes.size() > 10)
    m_frameTimes.pop_front();

  double frameRate = 1./(accumulate(m_frameTimes.begin(), m_frameTimes.end(), 0.) / m_frameTimes.size());
  if(m_showFrameRate)
    DrawFrameRate(frameRate);
}

void
GLWidget::SetLight(){

  //glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  GLfloat WhiteLight[] =  { 0.9f, 0.9f, 0.9f, 1.0f };
  glLightfv(GL_LIGHT0,GL_DIFFUSE,WhiteLight);
  glLightfv(GL_LIGHT1,GL_DIFFUSE,WhiteLight);

  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
}

void
GLWidget::SetLightPos() {
  static GLfloat lightPosition[] = { 250.0f, 250.0f, 250.0f, 1.0f };
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  static GLfloat lightPosition2[] = { -250.0f, 250.0f, -250.0f, 1.0f };
  glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
}

void
GLWidget::mousePressEvent(QMouseEvent* _e){

  SHIFT_CLICK = _e->modifiers() == Qt::ShiftModifier;

  //test camera motion first, then transform tool, then pick box
  if(GetCameraFactory().GetCurrentCamera()->MousePressed(_e)) {
    GetTransformTool().CameraMotion();
  }
  else if(!GetTransformTool().MousePressed(_e))
    GetPickBox().MousePressed(_e);

  updateGL();
}

void
GLWidget::mouseDoubleClickEvent(QMouseEvent* _e){
  GetVizmo().SetDoubleClickStatus(true);
  updateGL();
}

void
GLWidget::SimulateMouseUpSlot(){

  //simulate pick mouse up
  PickBox& pick = GetPickBox();
  QMouseEvent* e=NULL;
  pick.MouseReleased(e);

  updateGL();
}

void
GLWidget::mouseReleaseEvent(QMouseEvent* _e){
  bool handled = false;
  if(GetCameraFactory().GetCurrentCamera()->MouseReleased(_e)) {
    GetTransformTool().CameraMotion();
    handled = true;
  }
  else if(GetTransformTool().MouseReleased(_e))
    handled = true;

  if(handled){ //handled by gli
    updateGL();
    emit MRbyGLI();
    return;
  }

  //select
  PickBox& pick = GetPickBox();
  if(!m_takingSnapShot){
    if(pick.IsPicking()){
      pick.MouseReleased(_e);
      if(GetPickingFunction() != NULL){
        vector<Model*>& objs = GetPickedSceneObjs();
        //Shift key not pressed; discard current selection and start anew
        if(SHIFT_CLICK == false)
          objs.clear();
        //Get new set of picked objects if shift key not pressed
        //If shift is pressed, these are additional objects to add to
        //selection
        vector<Model*>& newobjs = GetPickingFunction()(pick.GetBox());
        objs.insert(objs.end(), newobjs.begin(), newobjs.end());

        if(SHIFT_CLICK == true)
          newobjs = objs;
        GetTransformTool().CheckSelectObject();
      }
    }
  }

  vector<Model*>& objs=GetVizmo().GetSelectedModels();
  if(_e->button() == Qt::RightButton){
    if(!objs.empty())
      emit selectByRMB();
    else //empty
      emit clickByRMB();
  }//not RMB
  else if(_e->button() == Qt::LeftButton){
    if(!objs.empty())
      emit selectByLMB();
    else
      emit clickByLMB();
  }

  //Update rotation of object
  /*if(objs.size()!=0){
    vector<Model*>& sel=GetVizmo().GetSelectedModels();
    typedef vector<Model*>::iterator OIT;
    for(OIT oit = sel.begin(); oit != sel.end(); oit++){
      if(((Model*)(*oit))->Name() != "Node") {
        typedef vector<Model*>::iterator GIT;
        MultiBodyModel* mbl;
        list<Model*> modelList;
        Model* gl;
        int i=0;
        for(GIT ig= GetVizmo().GetSelectedModels().begin();ig!=GetVizmo().GetSelectedModels().end();ig++){
          if(!modelList.empty()){
            i++;
            mbl=(MultiBodyModel*)(*ig);
            //get Polyhedron
            mbl->GetChildren(modelList);
            gl = modelList.front();

            //multiply polyhedron0 and multiBody quaternions
            //to get new rotation
            Quaternion finalQ = objs[0]->RotationQ() * gl->RotationQ();
            EulerAngle e;
            convertFromQuaternion(e, finalQ);

            mbl->Rotation()(e.alpha(), e.beta(), e.gamma());
          }
        }//end IF  ...actually, this appears to be end for -NJ
      }//end for
    }
  }*/

  updateGL();
}

void
GLWidget::mouseMoveEvent(QMouseEvent* _e){
  //test camera motion first, then transform tool, then pick box
  if(GetCameraFactory().GetCurrentCamera()->MouseMotion(_e)) {
    GetTransformTool().CameraMotion();
  }
  else if(!GetTransformTool().MouseMotion(_e))
    GetPickBox().MouseMotion(_e);

  updateGL();
}

void
GLWidget::keyPressEvent(QKeyEvent* _e) {
  //check camera then transform tool
  if(!GetCameraFactory().GetCurrentCamera()->KeyPressed(_e) &&
      !GetTransformTool().KeyPressed(_e))
    _e->ignore(); //not handled
  updateGL();
}

void
GLWidget::ShowAxis(){
  m_showAxis = !m_showAxis;
  updateGL();
}

void
GLWidget::ShowFrameRate(){
  m_showFrameRate = !m_showFrameRate;
  updateGL();
}

void
GLWidget::ResetTransTool(){
  GetTransformTool().ResetSelectedObj();
}

//save an image of the GL scene with the given filename
//Note: filename must have appropriate extension for QImage::save or no file
//will be written
void
GLWidget::SaveImage(QString _filename, bool _crop){
  //grab the gl scene. Copy into new QImage with size of imageRect. This will
  //crop the image appropriately.
  QRect imageRect = GetImageRect(_crop);
  QImage crop = grabFrameBuffer().copy(imageRect);
  crop.save(_filename);
}

//Grab the size of image for saving. If crop is true, use the cropBox to
//size the image down.
QRect
GLWidget::GetImageRect(bool _crop){
  if(_crop) {
    int xOff, yOff, w, h;
    PickBoxDim(&xOff, &yOff, &w, &h);
    return QRect(xOff+1, height()-yOff+1, w-2, h-2);
  }
  else
    return QRect(0, 0, width(), height());
}

void
GLWidget::DrawFrameRate(double _frameRate){
  glMatrixMode(GL_PROJECTION); //change to Ortho view
  glPushMatrix();
  glLoadIdentity();

  glOrtho(0, 20, 0, 20, -20, 20);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor3f(0, 0, 0);
  ostringstream oss;
  oss << "Theoretical Frame Rate: " << setw(5) << setprecision(4) << _frameRate;
  DrawStr(15.25, 19.25, 0, oss.str());

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

void
GLWidget::DrawAxis() {
  if(m_showAxis) {
    static GLuint gid = -1;
    if(gid == (GLuint)-1) { //no gid is created
      gid = glGenLists(1);
      glNewList(gid, GL_COMPILE);

      //create lines
      glLineWidth(2);
      glDisable(GL_LIGHTING);

      glBegin(GL_LINES);
      glColor3f(1,0,0);
      glVertex3f(0,0,0);
      glVertex3f(1,0,0);
      glColor3f(0,1,0);
      glVertex3f(0,0,0);
      glVertex3f(0,1,0);
      glColor3f(0,0,1);
      glVertex3f(0,0,0);
      glVertex3f(0,0,1);
      glEnd();

      //create letters
      glColor3f(1, 0, 0);
      DrawStr(1.25, 0, 0, "x");
      glColor3f(0, 1, 0);
      DrawStr(0, 1.25, 0, "y");
      glColor3f(0, 0, 1);
      DrawStr(0, 0, 1.25, "z");

      glEndList();
    }

    //draw reference axis
    glMatrixMode(GL_PROJECTION); //change to Ortho view
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, 20, 0, 20, -20, 20);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTranslated(1.2, 1.2, 0);
    Camera* cam = GetCameraFactory().GetCurrentCamera();
    glRotated(cam->GetCameraElev(), 1.0, 0.0, 0.0);
    glRotated(cam->GetCameraAzim(), 0.0, 1.0, 0.0);

    glCallList(gid);

    glPopMatrix();

    //pop to perspective view
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
  }
}

