#include "GLWidget.h"

#include <numeric>
#include <ctime>

#include <glut.h>
#include <GL/glu.h>

#include "MainWindow.h"

#include "Models/AvatarModel.h"
#include "Models/EnvModel.h"
#include "Models/RegionModel.h"
#include "Models/UserPathModel.h"
#include "Models/Vizmo.h"

#include "PHANToM/Manager.h"

#include "Utilities/Font.h"
#include "Utilities/GLUtils.h"

bool SHIFT_CLICK = false;

Q_DECLARE_METATYPE(Point3d);
int bs = qRegisterMetaType<Point3d>("Point3d");

///////////////////////////////////////////////////////////////////////////////
//This class handle opengl features

GLWidget::
GLWidget(QWidget* _parent) : QGLWidget(_parent),
    m_camera(Point3d(0, 0, 500), Vector3d(0, 0, 0)),
    m_transformTool(NULL), m_currentRegion(), m_currentUserPath(NULL) {
  setMinimumSize(271, 211); //original size: 400 x 600
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

  m_takingSnapShot=false;
  m_showAxis = true;
  m_showFrameRate = false;
  m_doubleClick = false;
  m_recording = false;

  connect(this, SIGNAL(SetMouse(Point3d)),
      this, SLOT(SetMousePosImpl(Point3d)));
}

void
GLWidget::
ToggleSelectionSlot() {
  m_takingSnapShot = !m_takingSnapShot;
}

void
GLWidget::
ResetCamera() {
  EnvModel* e = GetVizmo().GetEnv();
  if(e) {
    const auto& center = e->GetCenter();
    GetCurrentCamera()->Set(center + Point3d(0, 0, 1.5 * e->GetRadius()), center);
  }
  else
    GetCurrentCamera()->Set(Vector3d(0, 0, 1), Vector3d());
}

void
GLWidget::
initializeGL() {
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(1, 1, 1, 0);
  glEnable(GL_COLOR_MATERIAL);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glLineStipple(2, 0xAAAA);
}

void
GLWidget::
resizeGL(int _w, int _h) {
  GLUtils::windowWidth  = _w;
  GLUtils::windowHeight = _h;

  glViewport(0, 0, _w, _h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, ((GLfloat)_w)/((GLfloat)_h), 1, 10000);
}

void
GLWidget::
paintGL() {
  //start clock
  clock_t startTime = clock();

  //Render haptics!
  if(Haptics::UsingPhantom())
    GetVizmo().GetPhantomManager()->HapticRender();

  //Init Draw
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //GetMainWindow()->InitVizmo();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //draw camera
  GetCurrentCamera()->Draw();

  //draw pick box
  m_pickBox.Draw();

  //draw transform tool
  if(m_transformTool)
    m_transformTool->Draw();

  //draw cursor
  #ifdef USE_SPACEMOUSE
  m_cursor.Draw();
  #endif

  //draw axis
  DrawAxis();

  //set lights
  SetLights();

  //draw scene
  GetVizmo().Draw();

  //Render haptics!
  if(Haptics::UsingPhantom())
    GetVizmo().GetPhantomManager()->DrawRender();

  if(m_recording)
    emit Record();

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
GLWidget::
SetLights() {
  EnvModel* e = GetVizmo().GetEnv();
  if(e) {
    glEnable(GL_LIGHT0);
    float r = e->GetRadius();
    GLfloat lightPosition[] = {r, r, r, 1.f};
    GLfloat lightAmbient[] = {0.1f, 0.1f, 0.1f, 1.f};
    GLfloat lightDiffuse[] = {1.f, 1.f, 1.f, 1.f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  }
}

void
GLWidget::
mousePressEvent(QMouseEvent* _e) {

  SHIFT_CLICK = _e->modifiers() == Qt::ShiftModifier;

  //test camera motion first, then transform tool, then pick box
  if(!GetCurrentCamera()->MousePressed(_e) && !(m_transformTool && m_transformTool->MousePressed(_e))) {
    if((m_currentRegion && !m_currentRegion->MousePressed(_e, GetCurrentCamera()))
        || !m_currentRegion) {
      if((m_currentUserPath && !m_currentUserPath->MousePressed(_e, GetCurrentCamera())) ||
          !m_currentUserPath)
        m_pickBox.MousePressed(_e);
    }
  }
}

void
GLWidget::
mouseDoubleClickEvent(QMouseEvent* _e) {
  m_doubleClick = true;
}

void
GLWidget::
SimulateMouseUpSlot() {
  //simulate pick mouse up
  m_pickBox.MouseReleased(NULL);
}

void
GLWidget::
mouseReleaseEvent(QMouseEvent* _e) {
  bool handled = false;
  if(GetCurrentCamera()->MouseReleased(_e) || (m_transformTool && m_transformTool->MouseReleased(_e)))
    handled = true;
  else if(m_currentRegion)
    handled = m_currentRegion->MouseReleased(_e, GetCurrentCamera());
  else if(m_currentUserPath)
    handled = m_currentUserPath->MouseReleased(_e, GetCurrentCamera());

  if(handled){ //handled by gli
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
}


void
GLWidget::
mouseMoveEvent(QMouseEvent* _e) {
  //handle avatar
  if(GetVizmo().GetEnv())
    GetVizmo().GetEnv()->GetAvatar()->PassiveMouseMotion(_e, GetCurrentCamera());

  if(_e->buttons() == Qt::NoButton) {
    //handle all passive motion
    if(!(m_currentRegion && m_currentRegion->PassiveMouseMotion(_e,
        GetCurrentCamera())) &&
        !(m_currentUserPath && m_currentUserPath->PassiveMouseMotion(_e,
        GetCurrentCamera())))
      m_pickBox.PassiveMouseMotion(_e);
  }
  else {
    //handle active mouse motion
    //test camera motion first, then transform tool, then pick box
    if(!GetCurrentCamera()->MouseMotion(_e) && !(m_transformTool && m_transformTool->MouseMotion(_e))) {
      if((m_currentRegion
          && !m_currentRegion->MouseMotion(_e, GetCurrentCamera()))
          || !m_currentRegion ) {
        if((m_currentUserPath
            && !m_currentUserPath->MouseMotion(_e, GetCurrentCamera()))
            || !m_currentUserPath)
          m_pickBox.MouseMotion(_e);
      }
    }
  }
}


void
GLWidget::
keyPressEvent(QKeyEvent* _e) {
  //check for haptic toggle switch
  if(Haptics::UsingPhantom() && _e->key() == Qt::Key_QuoteLeft)
    GetVizmo().GetPhantomManager()->ToggleForceOutput();
  //check camera then transform tool
  else if(!GetCurrentCamera()->KeyPressed(_e) &&
      !(m_transformTool && m_transformTool->KeyPressed(_e)) &&
      (!m_currentUserPath || !m_currentUserPath->KeyPressed(_e)))
    _e->ignore(); //not handled
}


void
GLWidget::
ShowAxis() {
  m_showAxis = !m_showAxis;
}


void
GLWidget::
ShowFrameRate() {
  m_showFrameRate = !m_showFrameRate;
}

//save an image of the GL scene with the given filename
//Note: filename must have appropriate extension for QImage::save or no file
//will be written
void
GLWidget::
SaveImage(QString _filename, bool _crop) {
  //grab the gl scene. Copy into new QImage with size of imageRect. This will
  //crop the image appropriately.
  QRect imageRect = GetImageRect(_crop);
  QImage crop = grabFrameBuffer().copy(imageRect);
  crop.save(_filename);
}


//Grab the size of image for saving. If crop is true, use the cropBox to
//size the image down.
QRect
GLWidget::
GetImageRect(bool _crop) {
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
GLWidget::
DrawFrameRate(double _frameRate) {
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
GLWidget::
DrawAxis() {
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

    glTranslated(1.3, 1.3, 0);

    //figure out azim/elev of camera from at vector
    const Vector3d& dir = GetCurrentCamera()->GetDir();
    double azim = asind(Vector3d(1, 0, 0)*dir);
    if(dir[2] >= 0)
      azim = 180 - azim;
    double elev = -asind(Vector3d(0, 1, 0)*dir);
    glRotated(elev, 1, 0, 0);
    glRotated(azim, 0, 1, 0);

    glCallList(gid);

    glPopMatrix();

    //pop to perspective view
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
  }
}


void
GLWidget::
SetMousePosImpl(Point3d _p) {
  Point3d screenPos = GLUtils::ProjectToWindow(_p);
  QPoint globalPos = this->mapToGlobal(QPoint(screenPos[0],
      GLUtils::windowHeight - screenPos[1]));
  QCursor::setPos(globalPos);
}
