#ifndef _SCENE_WIN_H_
#define _SCENE_WIN_H_

/**
 * This file defines class for scene window of vimzo2. 
 * The scene window uses opengl widget.
 */

///////////////////////////////////////////////////////////////////////////////
#include <qgl.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGLWidget> 
// BSS
#include <GL/gli.h>
#include <list>
#include "TextGUI.h"
#include "MainWin.h"

using namespace std;

class TextGUI; 

class VizGLWin : public QGLWidget
{
  Q_OBJECT

  public:
    VizGLWin(QWidget* _parent = 0, VizmoMainWin* _mainWin = 0); 
    void resetCamera();
    VizmoMainWin* m_mainWin; 

    // BSS
    vector<gliObj>* objs2;

    void getWidthHeight(int*,int*);
    
    void setClearColor(double r, double g, double b) const {
      glClearColor(r, g, b, 0);
    }
    //Collision Detection related vars.
    bool CDOn;
    // reset tranformation tool
    // it calls class gli::gliReset()
    void resetTransTool();

  signals:
    void selectByRMB();
    void clickByRMB();
    void selectByLMB();
    void clickByLMB();
    void MRbyGLI();

  protected:
    void initializeGL();
    void resizeGL(int, int);
    void paintGL();
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent* _e);

  public slots:
    void showGrid();
    void showAxis();

  private slots:
    void toggleSelectionSlot();
    void getBoxDimensions(int*, int*, int*, int*); 
    void simulateMouseUpSlot();

  private:
    void SetLight();
    
    void 
    SetLightPos(){ //set light position
      
      static GLfloat light_position[] = { 250.0f, 250.0f, 250.0f, 1.0f };
      glLightfv(GL_LIGHT0, GL_POSITION, light_position);
      static GLfloat light_position2[] = { -250.0f, 250.0f, -250.0f, 1.0f };
      glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
    }

    bool takingSnapShot;
    bool m_bShowGrid, m_bShowAxis;

};


#endif /*_SCENE_WIN_H_ */
