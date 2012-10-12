#include "vizmo2.h"
#include "SceneWin.h"
#include "MainWin.h"
#include <GL/gli.h>
#include <GL/gliCamera.h>
#include <GL/gliFont.h>
#include <GL/glut.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGLWidget> 

///////////////////////////////////////////////////////////////////////////////
//This class handle opengl features

class VizmoMainWin; 

VizGLWin::VizGLWin(QWidget* _parent, VizmoMainWin* _mainWin)
  :QGLWidget(_parent)   
{
  m_mainWin = _mainWin; 
  setMinimumSize(400, 505); //original size: 400 x 600 
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding); 
  setFocusPolicy(Qt::StrongFocus);

  takingSnapShot=false;
  m_bShowGrid=false;
  m_bShowAxis=true;
}

void 
VizGLWin::getWidthHeight(int *w,int *h){

  *w=width();
  *h=height();
}

void 
VizGLWin::toggleSelectionSlot(){

  takingSnapShot=!takingSnapShot;
}

void 
VizGLWin::resetCamera(){

  double R=GetVizmo().GetEnvRadius();
  gliGetCameraFactory().getCurrentCamera()->setCameraPos(Point3d(0,0,4*R));
  gliGetCameraFactory().getCurrentCamera()->setAzim(0); 
  gliGetCameraFactory().getCurrentCamera()->setElev(0); 
}

//used as callback for gli
inline vector<gliObj>& 
vizmoSelect(const gliBox& _box){
  
  //calls CPlum::Select(const gliBox& box)
  GetVizmo().Select(_box); 
  return GetVizmo().GetSelectedItem();
}

void 
VizGLWin::initializeGL(){

  /*Setup light and material properties*/
  SetLight();

  /*others*/
  glEnable( GL_DEPTH_TEST);

  glClearColor( 1 , 1, 1, 0 );
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  glLineStipple(2,0xAAAA);

  //create models
  gliSetPickingFunction(vizmoSelect);
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
  if(m_bShowGrid) param=param|GLI_SHOW_GRID;
  if(m_bShowAxis) param=param|GLI_SHOW_AXIS;
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
VizGLWin::simulateMouseUpSlot(){

  gliSimMouseUp();
  updateGL();
}

void 
VizGLWin::getBoxDimensions(int *xOffset, int *yOffset,int *w,int *h){

  gliPickBoxDim(xOffset,yOffset,w,h);
}

void 
VizGLWin::mouseReleaseEvent(QMouseEvent* _e){

  if(gliMR(_e, takingSnapShot)){ //handled by gli
    updateGL(); 
    emit MRbyGLI();
    return; 
  }

  //updateGL();

  vector<gliObj>& objs=GetVizmo().GetSelectedItem();
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
    vector<gliObj>& sel=GetVizmo().GetSelectedItem();
    typedef vector<gliObj>::iterator OIT;
    for(OIT oit = sel.begin(); oit != sel.end(); oit++){
      if(((CGLModel*)(*oit))->GetName() != "Node") {
        typedef vector<gliObj>::iterator GIT;
        MultiBodyModel* mbl;
        list<CGLModel*> modelList;
        CGLModel* gl;
        int i=0;
        for(GIT ig= GetVizmo().GetSelectedItem().begin();ig!=GetVizmo().GetSelectedItem().end();ig++){
          if(!modelList.empty()){
            i++;
            mbl=(MultiBodyModel*)(*ig);
            //get Polyhedron
            mbl->GetChildren(modelList);
            gl = modelList.front();

            //get initial quaternion from polyhedron
            Quaternion qt0;
            qt0 = gl->q();
            //Quaternion qt0(gl->q());

            //get current/new rotation from objs var.
            Quaternion qrm;
            qrm = objs[0]->q();

            //multiply polyhedron0 and multiBody quaternions
            //to get new rotation
            Quaternion finalQ;
            finalQ = qrm * qt0;

            Matrix3x3 fm = finalQ.getMatrix();
            Vector3d fv = finalQ.MatrixToEuler(fm);

            mbl->rx() = fv[0];
            mbl->ry() = fv[1];
            mbl->rz() = fv[2];
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
    OBPRMView_Robot* rob = (OBPRMView_Robot*)(GetVizmo().GetRobot()->getModel());
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
  //if((OBPRMView_Robot*)(GetVizmo().GetRobot()) == NULL){
  if((GetVizmo().GetRobot()) == NULL){  //^casting presumably not necessary 
    updateGL(); 
    return;
  }
  if((OBPRMView_Robot*)(GetVizmo().GetRobot()->getModel())->KP(_e)){
    updateGL(); 
    return;
  }
  _e->ignore(); //not handled
}

void 
VizGLWin::showGrid() {

  m_bShowGrid=!m_bShowGrid; 
  updateGL();
}

void 
VizGLWin::showAxis() {

  m_bShowAxis=!m_bShowAxis; 
  updateGL();
}

void 
VizGLWin::resetTransTool(){
  
  gliReset();
}








