#ifndef REGION_SPHERE_MODEL_H_
#define REGION_SPHERE_MODEL_H_

#include "RegionModel.h"

#include "Models/CrosshairModel.h"

class Camera;

class RegionSphereModel : public RegionModel {
  public:
    enum Highlight {NONE, PERIMETER, ALL};

    RegionSphereModel(const Point3d& _center = Point3d(), double _radius = -1,
        bool _firstClick = true);

    shared_ptr<Boundary> GetBoundary() const;

    // Model functions
    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void Print(ostream& _os) const;
    // output debug information
    void OutputDebugInfo(ostream& _os) const;

    //mouse events for selecting and resizing
    bool MousePressed(QMouseEvent* _e, Camera* _c);
    bool MouseReleased(QMouseEvent* _e, Camera* _c);
    bool MouseMotion(QMouseEvent* _e, Camera* _c);
    bool PassiveMouseMotion(QMouseEvent* _e, Camera* _c);

    double WSpaceArea() const;

    // operators
    bool operator==(const RegionModel& _other) const;
    void ApplyOffset(const Vector3d& _v);
    void SetRadius(double _r) {m_radius = _r;}
    double GetShortLength() const;
    double GetLongLength() const;

  private:
    Point3d m_centerOrig;
    double m_radius, m_radiusOrig;

    bool m_lmb, m_rmb, m_firstClick;
    Highlight m_highlightedPart;
    QPoint m_clicked;

    CrosshairModel m_crosshair;
};

#endif
