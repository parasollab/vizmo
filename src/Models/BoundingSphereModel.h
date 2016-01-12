#ifndef BOUNDING_SPHERE_MODEL_H_
#define BOUNDING_SPHERE_MODEL_H_

#include <memory>
using namespace std;

#include "BoundaryModel.h"

class BoundingSphere;

////////////////////////////////////////////////////////////////////////////////
/// \brief Implements a spherical boundary.
////////////////////////////////////////////////////////////////////////////////
class BoundingSphereModel : public BoundaryModel {

  public:

    // Construction
    BoundingSphereModel(shared_ptr<BoundingSphere> _b);
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

    virtual size_t GetNumVertices() const override;
    virtual size_t GetNumFacets() const override;
    virtual void AddToTetGen(tetgenio* _tetModel, size_t _pOff, size_t _fOff) const override;

  private:
    shared_ptr<BoundingSphere> m_boundingSphere;
};

#endif
