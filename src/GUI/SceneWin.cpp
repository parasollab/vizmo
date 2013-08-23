#include "SceneWin.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QGLWidget>

#include <glut.h>

#include "Models/Vizmo.h"
#include "MainWin.h"

///////////////////////////////////////////////////////////////////////////////
//This class handle opengl features

class VizmoMainWin;

VizGLWin::VizGLWin(QWidget* _parent, VizmoMainWin* _mainWin)
  : QGLWidget(_parent){
    m_mainWin = _mainWin;
    setMinimumSize(400, 505); //original size: 400 x 600
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::StrongFocus);

    m_takingSnapShot=false;
    m_showGrid=false;
    m_showAxis=true;
    CDOn = false;
  }

void
VizGLWin::ToggleSelectionSlot(){

  m_takingSnapShot =! m_takingSnapShot;
}

void
VizGLWin::resetCamera(){
  gliGetCameraFactory().getCurrentCamera()->setCameraPos(Point3d(0, 0, 4*GetVizmo().GetEnvRadius()));
  gliGetCameraFactory().getCurrentCamera()->setAzim(0);
  gliGetCameraFactory().getCurrentCamera()->setElev(0);
}

//used as callback for gli
inline vector<GLModel*>&
VizmoSelect(const Box& _box){
  GetVizmo().Select(_box);
  return GetVizmo().GetSelectedModels();
}

void
VizGLWin::initializeGL(){

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
VizGLWin::resizeGL(int w, int h){
  gliWS(w,h);
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, ((GLfloat)w)/((GLfloat)h), 1, 1500);
}

void
VizGLWin::paintGL(){
  //Init Draw
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_mainWin->InitVizmo();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  int param=GLI_SHOW_PICKBOX|GLI_SHOW_TRANSFORMTOOL;
  if(m_showGrid) param=param|GLI_SHOW_GRID;
  if(m_showAxis) param=param|GLI_SHOW_AXIS;
  gliDraw(param);
  SetLightPos();
  GetVizmo().Display();
}

void
VizGLWin::SetLight(){

  //glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  GLfloat WhiteLight[] =  { 0.9f, 0.9f, 0.9f, 1.0f };
  glLightfv(GL_LIGHT0,GL_DIFFUSE,WhiteLight);
  glLightfv(GL_LIGHT1,GL_DIFFUSE,WhiteLight);

  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
}

void
VizGLWin::mousePressEvent(QMouseEvent* e){
  if( gliMP(e) ){
    updateGL();
    return;
  }//handled by gli

  updateGL();
}

void
VizGLWin::mouseDoubleClickEvent(QMouseEvent* _e){

  GetVizmo().SetDoubleClickStatus(true);
  updateGL();
}

void
VizGLWin::SimulateMouseUpSlot(){

  gliSimMouseUp();
  updateGL();
}

void
VizGLWin::mouseReleaseEvent(QMouseEvent* _e){
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
            Quaternion finalQ = objs[0]->q() * gl->q();
            EulerAngle e;
            convertFromQuaternion(e, finalQ);

            mbl->rx() = e.alpha();
            mbl->ry() = e.beta();
            mbl->rz() = e.gamma();
          }
        }//end IF  ...actually, this appears to be end for -NJ
      }//end for
    }
  }

  updateGL();
}

void
VizGLWin::mouseMoveEvent(QMouseEvent* e){
  if(gliMM(e)){
    //    if(CDOn)                   TEMPORARY(?) DISABLE
    //      GetVizmo().TurnOn_CD();
    updateGL();
    return;
  }//handled by gli

  updateGL();
}

void
VizGLWin::keyPressEvent (QKeyEvent* _e){

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
VizGLWin::showGrid() {
  m_showGrid = !m_showGrid;
  updateGL();
}

void
VizGLWin::showAxis() {
  m_showAxis = !m_showAxis;
  updateGL();
}

void
VizGLWin::resetTransTool(){
  gliReset();
}

//save an image of the GL scene with the given filename
//Note: filename must have appropriate extension for QImage::save or no file
//will be written
void
VizGLWin::SaveImage(QString _filename, bool _crop) {
  //grab the gl scene. Copy into new QImage with size of imageRect. This will
  //crop the image appropriately.
  QRect imageRect = GetImageRect(_crop);
  QImage crop = grabFrameBuffer().copy(imageRect);
  crop.save(_filename);
}

//Grab the size of image for saving. If crop is true, use the cropBox to
//size the image down.
QRect
VizGLWin::GetImageRect(bool _crop){
  if(_crop) {
    int xOff, yOff, w, h;
    gliPickBoxDim(&xOff, &yOff, &w, &h);
    return QRect(xOff+1, height()-yOff+1, w-2, h-2);
  }
  else
    return QRect(0, 0, width(), height());
}
