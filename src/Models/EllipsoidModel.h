#ifndef ELLIPSOID_MODEL_H
#define ELLIPSOID_MODEL_H

#include <memory>
using namespace std;

#include "../../../../../PMPL/trunk/utils/mathtool/Orientation.h"
#include "BoundaryModel.h"

//class NEllipsoid;

////////////////////////////////////////////////////////////////////////////////
/// \brief Implements a Elliptical Boundary.
////////////////////////////////////////////////////////////////////////////////
class EllipsoidModel : public BoundaryModel {

  public:

    // Construction
    EllipsoidModel();

    EllipsoidModel(const size_t _n, const std::vector<double>& _r,
         const EulerAngle& _e);

    EllipsoidModel(const vector<double>& _c, const vector<double>& _r,
        const EulerAngle& _e = EulerAngle());

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the boundary radius.
    virtual vector<double> GetRadii() const {return m_radii;}
    virtual vector<pair<double, double> > GetRanges() const {return m_axes;}

    /// Translate the entire n-sphere.
    /// @param _v The translation vector to apply.
    void Translate(const vector<double>& _v) noexcept;


    // BoundaryModel functions
    virtual double GetMaxDist();

    // Model functions
    virtual void Build();
    virtual void Print(ostream& _os) const;

  private:
    vector<double> m_center;  //Center
    vector<double> m_radii;  //radii in case of sphere and major/minor axes in case of an ellipseoid
    vector<pair<double, double> > m_axes; ///< The axis semi-ranges.
    EulerAngle m_orientation; ///< Orientation of the axes

};

#endif
