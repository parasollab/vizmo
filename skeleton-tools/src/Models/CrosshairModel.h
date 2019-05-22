#ifndef CROSSHAIR_MODEL_H_
#define CROSSHAIR_MODEL_H_

#include <QtCore>

#include "Model.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief A 3D crosshair to help users visualize depth when manipulating 3D
///        objects.
////////////////////////////////////////////////////////////////////////////////
class CrosshairModel : public Model {

  private:

    bool     m_enable{false}; ///< Indicates whether the crosshair is in use.
    Point3d* m_pos;           ///< Crosshair position.

  public:

    // Construction
    CrosshairModel(Point3d* _p = NULL) : Model("CrosshairModel"), m_pos(_p) {}

    // Crosshair position access
    void SetPos(Point3d& _p) {m_pos = &_p;} ///< Set the crosshair position.
    Point3d* GetPos() const {return m_pos;} ///< Get the crosshair position.

    // Control functions
    void Enable()  {m_enable = true;}       ///< Enable/disable the crosshair.
    void Disable() {m_enable = false;}      ///< Enable/disable the crosshair.

    // Model functions
    void Build() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect() {}
    void DrawSelected() {}
    void Print(ostream& _os) const {}
};

#endif
