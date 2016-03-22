#ifndef GL_WIDGET_H_
#define GL_WIDGET_H_

#include <deque>
#include <memory>

#include <QtGui>

#include "Utilities/Camera.h"
#include "Utilities/PickBox.h"
#include "Utilities/TransformTool.h"

#ifdef USE_SPACEMOUSE
#include "Utilities/Cursor3d.h"
#endif

using namespace std;

class MainWindow;
class RegionModel;
class UserPathModel;

////////////////////////////////////////////////////////////////////////////////
/// \brief  Creates and manages Vizmo's OpenGL scene.
////////////////////////////////////////////////////////////////////////////////
class GLWidget : public QGLWidget {

  Q_OBJECT

  public:

    GLWidget(QWidget* _parent);

    bool GetDoubleClickStatus() const {return m_doubleClick;}
    void SetDoubleClickStatus(bool _b) {m_doubleClick = _b;}

    void ResetCamera();
    Camera* GetCurrentCamera() {return &m_camera;}
    void SetMousePos(Point3d& _p) {emit SetMouse(_p);}

    void SetRecording(bool _b) {m_recording = _b;}

    void SetClearColor(double _r, double _g, double _b) const {
      glClearColor(_r, _g, _b, 0);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Activate a transform tool in the scene
    /// @param _tt Transform Tool to activate
    void SetTransformTool(TransformTool* _tt) {m_transformTool = _tt;}

    //save an image of the GL scene with the given filename
    //Note: filename must have appropriate extension for QImage::save or no file
    //will be written
    void SaveImage(QString _filename, bool _crop);

    shared_ptr<RegionModel> GetCurrentRegion() { return m_currentRegion;}
    void SetCurrentRegion(shared_ptr<RegionModel> _r = shared_ptr<RegionModel>()){
      m_currentRegion = _r;
    }

    UserPathModel* GetCurrentUserPath() {return m_currentUserPath;}
    void SetCurrentUserPath(UserPathModel* _p) {m_currentUserPath = _p;}

    #ifdef USE_SPACEMOUSE
    void ResetCursor() {m_cursor.Reset();}
    Cursor3d* GetCursor() {return &m_cursor;}
    #endif

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

    bool m_takingSnapShot;
    bool m_showAxis, m_showFrameRate;
    bool m_doubleClick;
    bool m_recording;

    deque<double> m_frameTimes;

    Camera m_camera;
    TransformTool* m_transformTool;
    PickBox m_pickBox;

    shared_ptr<RegionModel> m_currentRegion;
    UserPathModel* m_currentUserPath;

    #ifdef USE_SPACEMOUSE
    Cursor3d m_cursor{Point3d()};
    #endif
};

#endif
