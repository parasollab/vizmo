#ifndef REGION_BOX_MODEL_H_
#define REGION_BOX_MODEL_H_

#include "RegionModel.h"

#include "Models/CrosshairModel.h"

class Camera;

class RegionBoxModel : public RegionModel {
  public:
    enum Highlight {NONE = 0, LEFT = 1, RIGHT = 2, TOP = 4, BOTTOM = 8,
      FRONT = 16, BACK = 32, ALL = 63};

    RegionBoxModel();
    RegionBoxModel(pair<double, double> _xRange, pair<double, double> _yRange,
        pair<double, double> _zRange);

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
    // Output Debug Information
    void OutputDebugInfo(ostream& _os) const;

    //mouse events for selecting and resizing
    bool MousePressed(QMouseEvent* _e, Camera* _c);
    bool MouseReleased(QMouseEvent* _e, Camera* _c);
    bool MouseMotion(QMouseEvent* _e, Camera* _c);
    bool PassiveMouseMotion(QMouseEvent* _e, Camera* _c);

    //user feedback function
    double WSpaceArea() const;

    // operators
    bool operator==(const RegionModel& _other) const;
    void ApplyOffset(const Vector3d& _v);
    double GetShortLength() const;
    double GetLongLength() const;

  protected:
    //helper functions
    void FindCenter();
    void MapControls(Camera* _c, const Vector3d& _deltaMouse,
        Vector3d& _deltaWorld, vector<Vector3d>& _axisCtrlDir);
    void ApplyTransform(const Vector3d& _delta);

  private:
    //event tracking storage
    QPoint m_clicked;
    bool m_lmb, m_rmb, m_firstClick;
    int m_highlightedPart;

    //position storage
    vector<Vector3d> m_boxVertices;
    vector<Vector3d> m_prevPos;
    vector<Vector2d> m_winVertices;

    // More efficient saving of position
    // min and max are two points
    Point3d m_min;
    Point3d m_max;

    CrosshairModel m_crosshair;
};

#endif
