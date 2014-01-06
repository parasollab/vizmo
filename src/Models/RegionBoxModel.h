#ifndef REGIONBOXMODEL_H_
#define REGIONBOXMODEL_H_

#include "RegionModel.h"

class RegionBoxModel : public RegionModel {
  public:
    RegionBoxModel();

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
    double m_minx, m_maxx, m_miny, m_maxy;
};

#endif
