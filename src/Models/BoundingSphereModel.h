#ifndef BOUNDINGSPHEREMODEL_H_
#define BOUNDINGSPHEREMODEL_H_

#include "BoundaryModel.h"

class BoundingSphereModel : public BoundaryModel {
  public:
    BoundingSphereModel();
    BoundingSphereModel(const Point3d& _c, double _r);

    const Point3d& GetCenter() const {return m_center;}
    double GetRadius() const {return m_radius;}

    virtual vector<pair<double, double> > GetRanges();
    virtual bool Parse(istream& _is);
    virtual void Build();
    virtual void Print(ostream& _os) const;

  protected:
    virtual void Write(ostream& _os) const;

  private:
    Vector3d m_center;
    double m_radius;
};

#endif
