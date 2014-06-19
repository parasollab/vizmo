#ifndef USER_PATH_MODEL_H_
#define USER_PATH_MODEL_H_

#include "Model.h"
#include "Vizmo.h"

//class MultiBodyModel;

class UserPathModel : public Model {
  public:
    UserPathModel();

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

    //primative cfg extraction
    shared_ptr< vector<CfgModel> > GetCfgs();

    //mouse events for selecting and drawing
    bool MousePressed(QMouseEvent* _e, Camera* _c);
    bool MouseReleased(QMouseEvent* _e, Camera* _c);
    bool MouseMotion(QMouseEvent* _e, Camera* _c);
    bool PassiveMouseMotion(QMouseEvent* _e, Camera* _c) {return false;}

  protected:
    void UpdatePositions(const Point3d& _p);
    void UpdateValidity();

    vector<double> Point3dToVector(const Point3d& _p);

  private:
    bool m_finished, m_drawing;

    vector<Point3d> m_userPath;
    CfgModel m_oldPos;
    CfgModel m_newPos;
    bool m_valid;
};

#endif
