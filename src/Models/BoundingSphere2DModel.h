#ifndef BOUNDING_SPHERE_2D_MODEL_H_
#define BOUNDING_SPHERE_2D_MODEL_H_

#include <memory>
using namespace std;

#include "BoundaryModel.h"

class WorkspaceBoundingSphere;

////////////////////////////////////////////////////////////////////////////////
/// \brief Implements a spherical boundary.
////////////////////////////////////////////////////////////////////////////////
class BoundingSphere2DModel : public BoundaryModel {

  public:

    // Construction
    BoundingSphere2DModel(WorkspaceBoundingSphere* _b);
    BoundingSphere2DModel(const Point2d& _c, double _r);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the boundary radius.
    double GetRadius() const;

    // BoundaryModel functions
    virtual vector<pair<double, double> > GetRanges() const;
    virtual double GetMaxDist();

    // Model functions
    virtual void Build();
    virtual void Print(ostream& _os) const;

  private:
    WorkspaceBoundingSphere* m_boundingSphere;
};

#endif
