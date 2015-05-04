/*
 * This file defines class for the open gl scene of Vizmo.
 */

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include <deque>
#include <memory>

#include <QtGui>

#include "Utilities/Camera.h"
#include "Utilities/PickBox.h"
#include "Utilities/TransformTool.h"

using namespace std;

class MainWindow;
class RegionModel;
class UserPathModel;

class GLWidget : public QGLWidget {

  Q_OBJECT

  public:
    GLWidget(QWidget* _parent, MainWindow* _mainWindow);

    bool GetDoubleClickStatus() const {return m_doubleClick;}
    void SetDoubleClickStatus(bool _b) {m_doubleClick = _b;}

    void ResetCamera();
    Camera* GetCurrentCamera();
    void SetMousePos(Point3d& _p) {emit SetMouse(_p);}

    void SetRecording(bool _b) {m_recording = _b;}

    void SetClearColor(double _r, double _g, double _b) const {
      glClearColor(_r, _g, _b, 0);
    }
    // reset tranformation tool
    void ResetTransTool();

    //save an image of the GL scene with the given filename
    //Note: filename must have appropriate extension for QImage::save or no file
    //will be written
    void SaveImage(QString _filename, bool _crop);

    shared_ptr<RegionModel> GetCurrentRegion() { return m_currentRegion;}
    void SetCurrentRegion(shared_ptr<RegionModel> _r = shared_ptr<RegionModel>()) {m_currentRegion = _r;}

    UserPathModel* GetCurrentUserPath() {return m_currentUserPath;}
    void SetCurrentUserPath(UserPathModel* _p) {m_currentUserPath = _p;}

  signals:
    void selectByRMB();
    void clickByRMB();
    void selectByLMB();
    void clickByLMB();
    void MRbyGLI();
    void Record();
    void SetMouse(Point3d _p);

  private slots:
    void ShowAxis();
    void ShowFrameRate();
    void ToggleSelectionSlot();
    void SimulateMouseUpSlot();
    void SetMousePosImpl(Point3d _p);

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
    bool m_doubleClick;
    bool m_recording;

    deque<double> m_frameTimes;

    Camera m_camera;
    TransformTool m_transformTool;
    PickBox m_pickBox;

    shared_ptr<RegionModel> m_currentRegion;
    UserPathModel* m_currentUserPath;
};

#endif
