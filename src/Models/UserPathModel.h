#ifndef USER_PATH_MODEL_H_
#define USER_PATH_MODEL_H_

#include "Model.h"
#include "Vizmo.h"

class MainWindow;

class UserPathModel : public Model {

  public:
    //specify input type
    enum InputType {Mouse, Haptic, CameraPath};

    UserPathModel(MainWindow* _mainWindow, InputType _t = Mouse);

    //initialization of gl models
    void Build() {}
    //determing if _index is this GL model
    void Select(GLuint* _index, vector<Model*>& _sel);
    //draw is called for the scene.
    void DrawRender();
    void DrawSelect();
    //DrawSelect is only called if item is selected
    void DrawSelected();
    //output model info
    void Print(ostream& _os) const;
    void PrintPath(ostream& _os) const;

    //access functions
    const bool IsFinished() const {return m_finished;}
    const CfgModel& GetNewPos() const {return m_newPos;}
    const CfgModel& GetOldPos() const {return m_oldPos;}
    void RewindPos() {m_newPos = m_oldPos;}
    InputType GetInputType() {return m_type;}
    shared_ptr< vector<CfgModel> > GetCfgs();
    void SendToPath(const Point3d& _p);

    bool m_checkCollision;

    //mouse events for selecting and drawing
    bool MousePressed(QMouseEvent* _e, Camera* _c);
    bool MouseReleased(QMouseEvent* _e, Camera* _c);
    bool MouseMotion(QMouseEvent* _e, Camera* _c);
    bool PassiveMouseMotion(QMouseEvent* _e, Camera* _c) {return false;}
    bool KeyPressed(QKeyEvent* _e);

  protected:
    //helper functions
    void UpdatePositions(const Point3d& _p);
    void UpdateValidity();
    vector<double> Point3dToVector(const Point3d& _p);

  private:
    MainWindow* m_mainWindow;
    InputType m_type;
    bool m_finished, m_valid;
    CfgModel m_oldPos, m_newPos;
    vector<Point3d> m_userPath;
};

#endif
