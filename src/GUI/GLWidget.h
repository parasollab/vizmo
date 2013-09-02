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
    void resetCamera();
    MainWindow* m_mainWindow;

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
    void ShowAxis();
    void ShowFrameRate();

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

    void DrawFrameRate(double _frameRate);

    bool m_takingSnapShot;
    bool m_showAxis, m_showFrameRate;

    deque<double> m_frameTimes;
};

#endif
