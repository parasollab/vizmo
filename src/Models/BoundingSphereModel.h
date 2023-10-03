#ifndef BOUNDING_SPHERE_MODEL_H_
#define BOUNDING_SPHERE_MODEL_H_

#include <memory>
using namespace std;

#include "BoundaryModel.h"

class WorkspaceBoundingSphere;

////////////////////////////////////////////////////////////////////////////////
/// \brief Implements a spherical boundary.
////////////////////////////////////////////////////////////////////////////////
class BoundingSphereModel : public BoundaryModel {

  public:

    // Construction
    BoundingSphereModel(WorkspaceBoundingSphere* _b);
    BoundingSphereModel(const Point3d& _c, double _r);

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
