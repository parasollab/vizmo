#ifndef GLWIDGET_H_
#define GLWIDGET_H_

/**
 * This file defines class for the open gl scene of Vizmo.
 */

#include <deque>
using namespace std;

#include <QGLWidget>

#include "Utilities/Camera.h"
#include "Utilities/PickBox.h"
#include "Utilities/TransformTool.h"

class MainWindow;

class GLWidget : public QGLWidget {

  Q_OBJECT

  public:
    GLWidget(QWidget* _parent, MainWindow* _mainWindow);

    bool GetDoubleClickStatus() const {return m_doubleClick;}
    void SetDoubleClickStatus(bool _b) {m_doubleClick = _b;}

    void ResetCamera();
    Camera* GetCurrentCamera();

    void SetClearColor(double _r, double _g, double _b) const {
      glClearColor(_r, _g, _b, 0);
    }
    // reset tranformation tool
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

  private slots:
    void ShowAxis();
    void ShowFrameRate();
    void ToggleSelectionSlot();
    void SimulateMouseUpSlot();

  private:

    /////////////////////////////////////////////
    //overridden from qglwidget
    void initializeGL();
    void resizeGL(int _w, int _h);
    void paintGL();
    void mousePressEvent(QMouseEvent* _e);
    void mouseDoubleClickEvent(QMouseEvent* _e);
    void mouseReleaseEvent(QMouseEvent* _e);
    void mouseMoveEvent(QMouseEvent* _e);
    void keyPressEvent(QKeyEvent* _e);
    /////////////////////////////////////////////

    //setup for lighting
    void SetLight();
    void SetLightPos();

    //Grab the size of image for saving. If crop is true, use the cropBox to
    //size the image down.
    QRect GetImageRect(bool _crop);

    void DrawAxis();
    void DrawFrameRate(double _frameRate);

    MainWindow* m_mainWindow;

    bool m_takingSnapShot;
    bool m_showAxis, m_showFrameRate;
    //Collision Detection related vars.
    bool m_cdOn;
    bool m_doubleClick;

    deque<double> m_frameTimes;

    CameraFactory m_cameraFactory;
    TransformTool m_transformTool;
    PickBox m_pickBox;
};

#endif
