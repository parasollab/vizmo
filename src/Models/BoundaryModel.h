#ifndef BOUNDARY_MODEL_H_
#define BOUNDARY_MODEL_H_

#include <memory>
#include <string>

#include "Model.h"

class Boundary;

////////////////////////////////////////////////////////////////////////////////
/// \brief Provides a base class for drawable environment boundaries.
////////////////////////////////////////////////////////////////////////////////
class BoundaryModel : public Model {

  public:

    // Construction
    BoundaryModel(const string& _name, Boundary* _b);
    virtual ~BoundaryModel();

    // Model functions
    virtual void Build() = 0;
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    virtual void Print(ostream& _os) const = 0;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get minimums and maximums for this environment in X, Y, Z.
    virtual vector<pair<double, double> > GetRanges() const = 0;

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get maximum distance length for boundary
    virtual double GetMaxDist() = 0;

    const Point3d& GetCenter() const {return m_center;} ///< Get the center point.

    Boundary* GetBoundary() {return m_boundary;}

  protected:
    Boundary* m_boundary;

    size_t m_displayID; ///< This model's ID in the rendering call list.
    size_t m_linesID;   ///< This model's ID in the selection call list.

    Point3d m_center;   ///< The center of the boundary.
};

#endif
