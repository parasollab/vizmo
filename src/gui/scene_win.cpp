#include "vizmo2.h"
#include "scene_win.h"
#include "main_win.h"
#include <GL/gli.h>
#include <GL/gliCamera.h>
#include <GL/gliFont.h>
#include <GL/glut.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>

///////////////////////////////////////////////////////////////////////////////
//This class handle opengl features

   VizGLWin::VizGLWin(QWidget * parent, const char * name)
: QGLWidget(parent,name)
{ 
   setMinimumSize( 600, 400 );
   setFocusPolicy(Qt::StrongFocus);

   takingSnapShot=false;
   m_bShowGrid=m_bShowAxis=true;


}

void VizGLWin::getWidthHeight(int *w,int *h)
{
   *w=width();
   *h=height();
}

void VizGLWin::togleSlectionSlot()
{
   takingSnapShot=!takingSnapShot;
}

void VizGLWin::resetCamera()
{
   double R=GetVizmo().GetEnvRadius();
   gliGetCameraFactory().getCurrentCamera()->setCameraPos(Point3d(0,0,4*R));
}

//used as callback for gli
inline vector<gliObj>& vizmo_select(const gliBox& box){
   //calls CPlum::Select(const gliBox& box)
   GetVizmo().Select(box); 
   return GetVizmo().GetSelectedItem();
}

void VizGLWin::initializeGL()
{    
   /*Setup light and material properties*/
   SetLight();

   /*others*/
   glEnable( GL_DEPTH_TEST);

   glClearColor( 1 , 1, 1, 0 );
   //glEnable(GL_CULL_FACE);
   //glCullFace(GL_BACK);
   glLineStipple(2,0xAAAA);

   //create models
   gliSetPickingFunction(vizmo_select);
}

void VizGLWin::resizeGL( int w, int h)
{
   gliWS(w,h);
   glViewport( 0, 0, w, h );

   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   gluPerspective( 60, ((GLfloat)w)/((GLfloat)h), 1, 1500 );
}

void VizGLWin::paintGL()
{
   //Init Draw
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   ((VizmoMainWin*)parentWidget())->InitVizmo();

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   int param=GLI_SHOW_PICKBOX|GLI_SHOW_TRANSFORMTOOL;
   if( m_bShowGrid ) param=param|GLI_SHOW_GRID;
   if( m_bShowAxis ) param=param|GLI_SHOW_AXIS;
   gliDraw(param);
   SetLightPos();
   GetVizmo().Display();
   drawText();
}

void VizGLWin::SetLight()
{
   //glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);

   GLfloat WhiteLight[] =  { 0.9f, 0.9f, 0.9f, 1.0f };
   glLightfv(GL_LIGHT0,GL_DIFFUSE,WhiteLight);
   glLightfv(GL_LIGHT1,GL_DIFFUSE,WhiteLight);

   glEnable(GL_LIGHT0);
   glEnable(GL_LIGHT1);
}

void VizGLWin::mousePressEvent( QMouseEvent * e )
{
   if( gliMP(e) ){ updateGL(); return; }//handled by gli
   updateGL();
}


void VizGLWin::simulateMouseUpSlot()
{
   gliSimMouseUp();
   //  updateGL();
}

void VizGLWin::getBoxDimensions(int *xOffset, int *yOffset,int *w,int *h)
{
   gliPickBoxDim(xOffset,yOffset,w,h);
}

void VizGLWin::mouseReleaseEvent( QMouseEvent * e )
{ 
   if( gliMR(e,takingSnapShot) ){ //handled by gli
      updateGL(); 
      emit MRbyGLI();
      return; 
   }

   //updateGL();

   vector<gliObj>& objs=GetVizmo().GetSelectedItem();
   if( e->button()==Qt::RightButton ){
      if( !objs.empty() )
         emit selectByRMB();
      else //empty
         emit clickByRMB();
   }//not RMB
   else if( e->button()==Qt::LeftButton ){ 
      if( !objs.empty() ) 
         emit selectByLMB();

      else
         emit clickByLMB();

   }//if


   //Update rotation of object

   if(objs.size()!=0){
      vector<gliObj>& sel=GetVizmo().GetSelectedItem();
      typedef vector<gliObj>::iterator OIT;
      for(OIT i=sel.begin();i!=sel.end();i++){
         if( ((CGLModel*)(*i))->GetName()!="Node" ) {

            typedef vector<gliObj>::iterator GIT;
            CMultiBodyModel * mbl;
            list<CGLModel*> modelList;
            CGLModel* gl;
            int i=0;
            for(GIT ig= GetVizmo().GetSelectedItem().begin();ig!=GetVizmo().GetSelectedItem().end();ig++){
               if(!modelList.empty()){
                  i++;
                  mbl=(CMultiBodyModel*)(*ig);
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
            }//end IF
         }//end for
      }
   }

   updateGL();
}

void VizGLWin::mouseMoveEvent ( QMouseEvent * e )
{
   if( gliMM(e) ){  
      if(CDOn){
         GetVizmo().TurnOn_CD(); 
      }

      updateGL(); 
      return; 
   }//handled by gli

   updateGL();
}

void VizGLWin::keyPressEvent ( QKeyEvent * e )
{

#ifdef USE_PHANTOM
   //cout << "key" << endl;
   if(CDOn){
      OBPRMView_Robot * rob = (OBPRMView_Robot* )(GetVizmo().GetRobot()->getModel());
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
   if( gliCameraKEY(e) ){ updateGL();return; }
   if( gliKEY(e) ){ updateGL(); return; }//handled by gli
   if( (OBPRMView_Robot* )(GetVizmo().GetRobot()->getModel())->KP(e) ){updateGL(); return;}
   e->ignore(); //not handled
}

void VizGLWin::drawText()
{
   vector<gliObj>& sel=GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator SIT;

   glPushAttrib(GL_CURRENT_BIT);

   //draw reference axis
   glMatrixMode(GL_PROJECTION); //change to Ortho view
   glPushMatrix(); 
   glLoadIdentity();
   gluOrtho2D(0,20,0,20);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glDisable(GL_LIGHTING);

   //Draw text
   glTranslated(0,20,0);
   for(SIT i=sel.begin();i!=sel.end();i++){
      CGLModel * gl=(CGLModel *)(*i);

      //*********** this is the original code **********
      //list<string> info=gl->GetInfo();
      //drawText(info);
      // ***********************************************
      //June 08-05
      if(gl != NULL){
         list<string> info=gl->GetInfo();
         //NOTE:: comment out next line
         //if( gl->GetName()!="MultiBody" )
         drawText(info);
      }
   }

   glPopMatrix();

   //pop GL_PROJECTION
   glMatrixMode(GL_PROJECTION); //change to Pers view
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopAttrib();
}

void VizGLWin::drawText(list<string>& info)
{
   typedef list<string>::iterator SIT;

   for(SIT i=info.begin();i!=info.end();i++){
      //////////////////////////////////////////////
      glTranslated(0,-0.5,0);
      int pos = i->find("**", 0); 
      if (pos == string::npos)
         glColor3f(0.2,0.2,0.5);
      else
         glColor3f(1, 0, 0);

      drawstr(0.2,0,0,i->c_str());

   }
}

void VizGLWin::showGrid() 
{ 
   m_bShowGrid=!m_bShowGrid; 
   updateGL();
}

void VizGLWin::showAxis() 
{ 
   m_bShowAxis=!m_bShowAxis; 
   updateGL();
}

void VizGLWin::resetTransTool()
{
   gliReset();
}








