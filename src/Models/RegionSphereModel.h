#ifndef REGIONSPHEREMODEL_H_
#define REGIONSPHEREMODEL_H_

#include "RegionModel.h"

class RegionSphereModel : public RegionModel {
  public:
    RegionSphereModel();

    shared_ptr<Boundary> GetBoundary() const;

    //initialization of gl models
    void BuildModels();
    //determing if _index is this GL model
    void Select(GLuint* _index, vector<Model*>& _sel);
    //draw is called for the scene.
    void Draw();
    //DrawSelect is only called if item is selected
    void DrawSelect();
    //output model info
    void Print(ostream& _os) const;

    //mouse events for selecting and resizing
    bool MousePressed(QMouseEvent* _e);
    bool MouseReleased(QMouseEvent* _e);
    bool MouseMotion(QMouseEvent* _e);
    bool PassiveMouseMotion(QMouseEvent* _e);

  private:
    Vector3d m_center;
    double m_radius;
};

#endif
