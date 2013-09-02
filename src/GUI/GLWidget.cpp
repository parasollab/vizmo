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
    CDOn = false;
  }

void
GLWidget::ToggleSelectionSlot(){
  m_takingSnapShot = !m_takingSnapShot;
}

void
GLWidget::resetCamera(){
  GetCameraFactory().GetCurrentCamera()->Set(Point3d(0, 0, 4*GetVizmo().GetEnvRadius()), 0, 0);
}

//used as callback for gli
inline vector<GLModel*>&
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
  gliSetPickingFunction(VizmoSelect);
}

void
GLWidget::resizeGL(int w, int h){
  gliWS(w,h);
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, ((GLfloat)w)/((GLfloat)h), 1, 1500);
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

  int param = GLI_SHOW_PICKBOX | GLI_SHOW_TRANSFORMTOOL;
  if(m_showAxis) param = param | GLI_SHOW_AXIS;

  gliDraw(param);
  SetLightPos();
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
GLWidget::mousePressEvent(QMouseEvent* e){
  if( gliMP(e) ){
    updateGL();
    return;
  }//handled by gli

  updateGL();
}

void
GLWidget::mouseDoubleClickEvent(QMouseEvent* _e){

  GetVizmo().SetDoubleClickStatus(true);
  updateGL();
}

void
GLWidget::SimulateMouseUpSlot(){

  gliSimMouseUp();
  updateGL();
}

void
GLWidget::mouseReleaseEvent(QMouseEvent* _e){
  if(gliMR(_e, m_takingSnapShot)){ //handled by gli
    updateGL();
    emit MRbyGLI();
    return;
  }

  //updateGL();

  vector<GLModel*>& objs=GetVizmo().GetSelectedModels();
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
  if(objs.size()!=0){
    vector<GLModel*>& sel=GetVizmo().GetSelectedModels();
    typedef vector<GLModel*>::iterator OIT;
    for(OIT oit = sel.begin(); oit != sel.end(); oit++){
      if(((GLModel*)(*oit))->GetName() != "Node") {
        typedef vector<GLModel*>::iterator GIT;
        MultiBodyModel* mbl;
        list<GLModel*> modelList;
        GLModel* gl;
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
  }

  updateGL();
}

void
GLWidget::mouseMoveEvent(QMouseEvent* e){
  if(gliMM(e)){
    //    if(CDOn)                   TEMPORARY(?) DISABLE
    //      GetVizmo().TurnOn_CD();
    updateGL();
    return;
  }//handled by gli

  updateGL();
}

void
GLWidget::keyPressEvent (QKeyEvent* _e){

#ifdef USE_PHANTOM
  //cout << "key" << endl;
  if(CDOn){
    RobotModel* rob = (RobotModel*)(GetVizmo().GetRobot()->getModel());
    //cout << "on " << endl;
    GetPhantomManager().CDOn = true;
    GetVizmo().TurnOn_CD();
    double x = rob -> gettx();
    double y = rob -> getty();
    double z = rob -> gettz();
    //cout << x << " " << y << " " << z << endl;
    GetPhantomManager().fpos.clear();
    GetPhantomManager().fpos.push_back(x);
    GetPhantomManager().fpos.push_back(y);
    GetPhantomManager().fpos.push_back(z);
    GetPhantomManager().proceed = true;
  }
  else
    GetPhantomManager().CDOn = false;
#endif

  updateGL();
  if(gliCameraKEY(_e)){
    updateGL();
    return;
  }
  if(gliKEY(_e)){
    updateGL();
    return;
  }//handled by gli
  if(!GetVizmo().GetRobot()) {
    updateGL();
    return;
  }
  if(GetVizmo().GetRobot()->KP(_e)){
    updateGL();
    return;
  }
  _e->ignore(); //not handled
}

void
GLWidget::ShowAxis() {
  m_showAxis = !m_showAxis;
  updateGL();
}

void
GLWidget::ShowFrameRate() {
  m_showFrameRate = !m_showFrameRate;
  updateGL();
}

void
GLWidget::resetTransTool(){
  gliReset();
}

//save an image of the GL scene with the given filename
//Note: filename must have appropriate extension for QImage::save or no file
//will be written
void
GLWidget::SaveImage(QString _filename, bool _crop) {
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
    gliPickBoxDim(&xOff, &yOff, &w, &h);
    return QRect(xOff+1, height()-yOff+1, w-2, h-2);
  }
  else
    return QRect(0, 0, width(), height());
}

void
GLWidget::DrawFrameRate(double _frameRate) {
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
