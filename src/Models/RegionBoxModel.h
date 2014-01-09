#ifndef REGIONBOXMODEL_H_
#define REGIONBOXMODEL_H_

#include "RegionModel.h"

class RegionBoxModel : public RegionModel {
  public:
    enum Highlight {NONE = 0, LEFT = 1, RIGHT = 2, TOP = 4, BOTTOM = 8, ALL = 16};

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
    //vertex storage
    vector<Vector3d> m_boxVertices;
    vector<Vector3d> m_prevPos;

    //event tracking storage
    QPoint m_clicked;
    bool m_lmb, m_firstClick;
    int m_highlightedPart;
};

#endif
