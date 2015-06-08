#ifndef CROSSHAIR_MODEL_H_
#define CROSSHAIR_MODEL_H_

#include "Model.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Implements a 3D crosshair to help users visualize depth when
///        manipulating 3D objects.
////////////////////////////////////////////////////////////////////////////////
class CrosshairModel : public Model {

  public:

    // Construction
    CrosshairModel(Point3d* _p = NULL);

    // Crosshair position access
    void SetPos(Point3d& _p) {m_worldPos = &_p;} ///< Set the crosshair position.
    Point3d* GetPos() const {return m_worldPos;} ///< Get the crosshair position.

    // Control functions
    void Toggle() {m_enabled = !m_enabled;}    ///< Enable/disable the crosshair.

    // Model functions
    void Build() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect() {}
    void DrawSelected() {}
    void Print(ostream& _os) const {}

  private:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Determines whether the crosshair is inside the environment limits.
    bool IsInsideBBX() const;

    Point3d* m_worldPos;                         ///< Crosshair position.
    vector< pair<double, double> > m_worldRange; ///< Environment limits.
    bool m_enabled;       ///< Indicates whether the crosshair is in use.
};

#endif
