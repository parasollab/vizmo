#ifndef REGIONSPHERE2DMODEL_H_
#define REGIONSPHERE2DMODEL_H_

#include "RegionModel.h"

class Camera;

class RegionSphere2DModel : public RegionModel {
  public:
    enum Highlight {NONE, PERIMETER, ALL};

    RegionSphere2DModel(const Point3d& _center = Point3d(), double _radius = -1, bool _firstClick = true);

    shared_ptr<Boundary> GetBoundary() const;

    //initialization of gl models
    void Build();
    //determing if _index is this GL model
    void Select(GLuint* _index, vector<Model*>& _sel);
    //draw is called for the scene.
    void DrawRender();
    void DrawSelect();
    //DrawSelect is only called if item is selected
    void DrawSelected();
    //output model info
    void Print(ostream& _os) const;
    // output debug info
    void OutputDebugInfo(ostream& _os) const;

    //mouse events for selecting and resizing
    bool MousePressed(QMouseEvent* _e, Camera* _c);
    bool MouseReleased(QMouseEvent* _e, Camera* _c);
    bool MouseMotion(QMouseEvent* _e, Camera* _c);
    bool PassiveMouseMotion(QMouseEvent* _e, Camera* _c);

    double WSpaceArea() const;

    const bool operator==(const RegionModel& _other) const;

  private:
    Vector3d m_center, m_centerOrig;
    double m_radius, m_radiusOrig;

    bool m_lmb, m_firstClick;
    Highlight m_highlightedPart;
    QPoint m_clicked;

    RegionSphere2DModel* myPrevModel;
};

#endif
