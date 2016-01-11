#ifndef BOUNDARY_MODEL_H_
#define BOUNDARY_MODEL_H_

#include <memory>
#include <string>

#include "Model.h"

#define TETLIBRARY
#include "tetgen1.5.0/tetgen.h"

class Boundary;

////////////////////////////////////////////////////////////////////////////////
/// \brief Provides a base class for drawable environment boundaries.
////////////////////////////////////////////////////////////////////////////////
class BoundaryModel : public Model {

  public:

    // Construction
    BoundaryModel(const string& _name, shared_ptr<Boundary> _b);
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

    shared_ptr<Boundary>& GetBoundary() {return m_boundary;}

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Number of vertices in boundary model
    virtual size_t GetNumVertices() const = 0;

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Number of facets in boundary model
    virtual size_t GetNumFacets() const = 0;

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Add boundary model to tetgen
    virtual void AddToTetGen(tetgenio* _tetModel) const = 0;

  protected:
    shared_ptr<Boundary> m_boundary;

    size_t m_displayID; ///< This model's ID in the rendering call list.
    size_t m_linesID;   ///< This model's ID in the selection call list.

    Point3d m_center;   ///< The center of the boundary.
};

#endif
