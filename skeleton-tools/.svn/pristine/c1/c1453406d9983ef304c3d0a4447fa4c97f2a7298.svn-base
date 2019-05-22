#ifndef BOUNDING_SPHERE_2D_MODEL_H_
#define BOUNDING_SPHERE_2D_MODEL_H_

#include <memory>
using namespace std;

#include "BoundaryModel.h"

class BoundingSphere2D;

////////////////////////////////////////////////////////////////////////////////
/// \brief Implements a spherical boundary.
////////////////////////////////////////////////////////////////////////////////
class BoundingSphere2DModel : public BoundaryModel {

  public:

    // Construction
    BoundingSphere2DModel(shared_ptr<BoundingSphere2D> _b);
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
    shared_ptr<BoundingSphere2D> m_boundingSphere;
};

#endif
