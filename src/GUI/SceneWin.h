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
#include <Utilities/GL/gli.h>
#include <list>
#include "TextGUI.h"
#include "MainWin.h"

using namespace std;

class TextGUI;

class VizGLWin : public QGLWidget {
  Q_OBJECT

  public:
    VizGLWin(QWidget* _parent = 0, VizmoMainWin* _mainWin = 0);
    void resetCamera();
    VizmoMainWin* m_mainWin;

    void setClearColor(double r, double g, double b) const {
      glClearColor(r, g, b, 0);
    }
    //Collision Detection related vars.
    bool CDOn;
    // reset tranformation tool
    // it calls class gli::gliReset()
    void resetTransTool();

    //save an image of the GL scene with the given filename
    //Note: filename must have appropriate extension for QImage::save or no file
    //will be written
    void SaveImage(QString _filename, bool _crop);

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
    void mouseDoubleClickEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent* _e);

  public slots:
    void showAxis();

  private slots:
    void ToggleSelectionSlot();
    void SimulateMouseUpSlot();

  private:
    void SetLight();

    void
    SetLightPos(){ //set light position

      static GLfloat light_position[] = { 250.0f, 250.0f, 250.0f, 1.0f };
      glLightfv(GL_LIGHT0, GL_POSITION, light_position);
      static GLfloat light_position2[] = { -250.0f, 250.0f, -250.0f, 1.0f };
      glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
    }

    //Grab the size of image for saving. If crop is true, use the cropBox to
    //size the image down.
    QRect GetImageRect(bool _crop);

    bool m_takingSnapShot;
    bool m_showAxis;

};

#endif /*_SCENE_WIN_H_ */
