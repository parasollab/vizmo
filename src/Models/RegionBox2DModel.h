#ifndef REGIONBOX2DMODEL_H_
#define REGIONBOX2DMODEL_H_

#include "RegionModel.h"

class Camera;

class RegionBox2DModel : public RegionModel {
  public:
    enum Highlight {NONE = 0, LEFT = 1, RIGHT = 2, TOP = 4, BOTTOM = 8, ALL = 15};

    RegionBox2DModel();

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

    //mouse events for selecting and resizing
    bool MousePressed(QMouseEvent* _e, Camera* _c);
    bool MouseReleased(QMouseEvent* _e, Camera* _c);
    bool MouseMotion(QMouseEvent* _e, Camera* _c);
    bool PassiveMouseMotion(QMouseEvent* _e, Camera* _c);

    //user feedback function
    double WSpaceArea() const;

  protected:
    //helper function
    void FindCenter();
    void ApplyTransform(const Vector2d& _delta);

  private:
    //event tracking storage
    QPoint m_clicked;
    bool m_lmb, m_firstClick;
    int m_highlightedPart;

    //position storage
    vector<Vector3d> m_boxVertices;
    vector<Vector3d> m_prevPos;
    vector<Vector2d> m_winVertices;
    Point3d m_center;
};

#endif
