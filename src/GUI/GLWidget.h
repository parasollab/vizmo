#ifndef GLWIDGET_H_
#define GLWIDGET_H_

/**
 * This file defines class for the open gl scene of Vizmo.
 */

#include <deque>
using namespace std;

#include <qgl.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGLWidget>

#include <Utilities/GL/gli.h>
#include "TextGUI.h"

class MainWindow;
class TextGUI;

class GLWidget : public QGLWidget {

  Q_OBJECT

  public:
    GLWidget(QWidget* _parent, MainWindow* _mainWindow);
    void ResetCamera();
    MainWindow* m_mainWindow;

    void SetClearColor(double _r, double _g, double _b) const {
      glClearColor(_r, _g, _b, 0);
    }
    // reset tranformation tool
    // it calls class gli::gliReset()
    void ResetTransTool();

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

  private slots:
    void ShowAxis();
    void ShowFrameRate();
    void ToggleSelectionSlot();
    void SimulateMouseUpSlot();

  private:
    void SetLight();
    void SetLightPos(){ //set light position
      static GLfloat lightPosition[] = { 250.0f, 250.0f, 250.0f, 1.0f };
      glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
      static GLfloat lightPosition2[] = { -250.0f, 250.0f, -250.0f, 1.0f };
      glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
    }
    //Grab the size of image for saving. If crop is true, use the cropBox to
    //size the image down.
    QRect GetImageRect(bool _crop);

    void DrawFrameRate(double _frameRate);

    bool m_takingSnapShot;
    bool m_showAxis, m_showFrameRate;
    //Collision Detection related vars.
    bool m_cdOn;
    deque<double> m_frameTimes;
};

#endif
