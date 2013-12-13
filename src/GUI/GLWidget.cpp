#include "GLWidget.h"

#include <numeric>
#include <ctime>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QGLWidget>

#include <glut.h>
#include <GL/glu.h>

#include "MainWindow.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"
#include "Utilities/Camera.h"
#include "Utilities/Font.h"
#include "Utilities/TransformTool.h"

bool SHIFT_CLICK = false;

///////////////////////////////////////////////////////////////////////////////
//This class handle opengl features

GLWidget::GLWidget(QWidget* _parent, MainWindow* _mainWindow)
  : QGLWidget(_parent), m_transformTool(m_cameraFactory.GetCurrentCamera()) {
    m_mainWindow = _mainWindow;
    setMinimumSize(271, 211); //original size: 400 x 600
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::StrongFocus);

    m_takingSnapShot=false;
    m_showAxis = true;
    m_showFrameRate = false;
    m_cdOn = false;
    m_doubleClick = false;
}

void
GLWidget::ToggleSelectionSlot(){
  m_takingSnapShot = !m_takingSnapShot;
}

void
GLWidget::ResetCamera(){
  EnvModel* e = GetVizmo().GetEnv();
  GetCurrentCamera()->Set(Vector3d(0,0,4*(e ? e->GetRadius() : 100)),Vector3d(0,0,0), Vector3d(0,1,0));
}

Camera*
GLWidget::GetCurrentCamera() {
  return m_cameraFactory.GetCurrentCamera();
}

void
GLWidget::initializeGL(){
  //Setup light and material properties
  SetLight();

  glEnable(GL_DEPTH_TEST);

  glClearColor(1, 1, 1, 0);
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  glLineStipple(2, 0xAAAA);
}

void
GLWidget::resizeGL(int _w, int _h) {
  m_transformTool.SetWindowSize(_w, _h);
  m_pickBox.SetWinSize(_w, _h);

  glViewport(0, 0, _w, _h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, ((GLfloat)_w)/((GLfloat)_h), 1, 10000);
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
  GetCurrentCamera()->Draw();

  //draw pick box
  m_pickBox.Draw();

  //draw transform tool
  m_transformTool.Draw();

  //draw axis
  DrawAxis();

  //set lights
  SetLightPos();

  //draw scene
  GetVizmo().Draw();

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
  if(GetCurrentCamera()->MousePressed(_e)) {
    m_transformTool.CameraMotion();
  }
  else if(!m_transformTool.MousePressed(_e))
    m_pickBox.MousePressed(_e);

  updateGL();
}

void
GLWidget::mouseDoubleClickEvent(QMouseEvent* _e) {
  m_doubleClick = true;
}

void
GLWidget::SimulateMouseUpSlot(){

  //simulate pick mouse up
  m_pickBox.MouseReleased(NULL);

  updateGL();
}

void
GLWidget::mouseReleaseEvent(QMouseEvent* _e){
  bool handled = false;
  if(GetCurrentCamera()->MouseReleased(_e)) {
    m_transformTool.CameraMotion();
    handled = true;
  }
  else if(m_transformTool.MouseReleased(_e))
    handled = true;

  if(handled){ //handled by gli
    updateGL();
    emit MRbyGLI();
    return;
  }

  //select
  PickBox& pick = m_pickBox;
  if(!m_takingSnapShot) {
    if(pick.IsPicking()) {
      pick.MouseReleased(_e);

      static vector<Model*> objs;
      if(!SHIFT_CLICK)
        objs.clear();

      //Get new set of picked objects if shift key not pressed
      //If shift is pressed, these are additional objects to add to
      //selection
      GetVizmo().Select(m_pickBox.GetBox());
      vector<Model*>& newObjs = GetVizmo().GetSelectedModels();
      if((SHIFT_CLICK)&&(newObjs.size()==1)){
        vector<int> toErase[2];
        typedef vector<Model*>::iterator SI;
        int u=0;
        for(SI i = objs.begin(); i!= objs.end(); i++){
          int v=0;
          for(SI j = newObjs.begin(); j!= newObjs.end(); j++){
            if((*i)==(*j)){
              toErase[0].push_back(u);
              toErase[1].push_back(v);
            }
            u++;
          }
          v++;
        }
        while(toErase[0].size()!=0){
          objs.erase(objs.begin()+toErase[0].back());
          toErase[0].pop_back();
        }
        while(toErase[1].size()!=0){
          newObjs.erase(newObjs.begin()+toErase[1].back());
          toErase[1].pop_back();
        }
      }
      if(newObjs.size()!=0)
        objs.insert(objs.end(), newObjs.begin(), newObjs.end());

      if(SHIFT_CLICK)
        newObjs = objs;

      m_transformTool.CheckSelectObject();
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
  if(GetCurrentCamera()->MouseMotion(_e)) {
    m_transformTool.CameraMotion();
  }
  else if(!m_transformTool.MouseMotion(_e))
    m_pickBox.MouseMotion(_e);

  updateGL();
}

void
GLWidget::keyPressEvent(QKeyEvent* _e) {
  //check camera then transform tool
  if(!GetCurrentCamera()->KeyPressed(_e) &&
      !m_transformTool.KeyPressed(_e))
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
  m_transformTool.ResetSelectedObj();
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
    // handle all the ways the pick box can be drawn
    const Box& box = m_pickBox.GetBox();
    int xOff = min(box.m_left, box.m_right);
    int yOff = max(box.m_bottom, box.m_top);
    int w = abs(box.m_right - box.m_left);
    int h = abs(box.m_bottom - box.m_top);

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
    Camera* cam = GetCurrentCamera();
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

