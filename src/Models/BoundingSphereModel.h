#ifndef BOUNDING_SPHERE_MODEL_H_
#define BOUNDING_SPHERE_MODEL_H_

#include "BoundaryModel.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Implements a spherical boundary.
////////////////////////////////////////////////////////////////////////////////
class BoundingSphereModel : public BoundaryModel {

  public:

    // Construction
    BoundingSphereModel();
    BoundingSphereModel(const Point3d& _c, double _r);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the boundary radius.
    double GetRadius() const {return m_radius;}

    // BoundaryModel functions
    virtual vector<pair<double, double> > GetRanges();
    virtual bool Parse(istream& _is);

    // Model functions
    virtual void Build();
    virtual void Print(ostream& _os) const;

  protected:

    virtual void Write(ostream& _os) const;

  private:

    double m_radius; ///< The boundary radius.
};

#endif
